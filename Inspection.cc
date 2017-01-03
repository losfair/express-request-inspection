#include <thread>
#include <mutex>
#include <chrono>
#include <map>
#include <list>

#include <stdio.h>
#include <time.h>

#include "Inspection.h"

#define CONSOLE_OUTPUT_PREFIX "[express-request-inspection] "

namespace Inspection {
    // Maximum 50 requests every 10 seconds by default.
    static unsigned int REQUEST_TIME_WINDOW_SIZE = 50;
    static unsigned int MIN_TIME_DIFF_PER_WINDOW = 10; // Minimum time difference allowed between the first and last requests in one window, in seconds.
    static unsigned int RULE_EXPIRE_TIME = 300; // Time in seconds for a rule to expire.
    static unsigned int REQUEST_EXPIRE_TIME = 300; // Time in seconds for a request to expire.
    
    static std::list<RequestInfo> *requestInfoQueuePtr = NULL;
    static std::thread *inspectionThread = NULL;
    static bool workerNotification = false;
    static std::mutex workerLock;
    static std::map<std::string, RequestSourceRule> rules;
    static std::map<std::string, std::list<time_t> > requestTimeWindows;

    RequestSourceRule::RequestSourceRule() {
        src.ip = "";
        isAllowed = true;
        createTime = time(0);
    }

    RequestSourceRule::RequestSourceRule(RequestSource _src, bool _isAllowed) {
        src = _src;
        isAllowed = _isAllowed;
        createTime = time(0);
    }

    RequestSourceRule::RequestSourceRule(std::string srcIp, bool _isAllowed) {
        src.ip = srcIp;
        isAllowed = _isAllowed;
        createTime = time(0);
    }

    std::string RequestSource::getUniqueIdentifier() {
        return ip; // Currently only IP is used to inspect the request.
    }

    std::string RequestSourceRule::getUniqueIdentifier() {
        return src.getUniqueIdentifier();
    }

    bool checkRequest(RequestInfo& info) {
        if(info.find("ip") == info.end()) {
            fprintf(stderr, CONSOLE_OUTPUT_PREFIX "Warning: No IP provided\n");
            return true;
        }

        RequestSource src;
        src.ip = info["ip"];

        auto itr = rules.find(src.getUniqueIdentifier());
        if(itr == rules.end()) return true;

        if(!itr -> second.isAllowed) {
            return false;
        }

        return true;
    }

    void addRequestInfoToQueue(RequestInfo& reqInfo) {
        if(!requestInfoQueuePtr) return;

        workerLock.lock();

        auto& requestInfoQueue = *requestInfoQueuePtr;
        requestInfoQueue.push_back(reqInfo);
        workerNotification = true; // Notify the inspection worker,

        workerLock.unlock();
    }

    void inspectRequestIp(std::string& ip) {
        if(ip.empty()) {
            fprintf(stderr, CONSOLE_OUTPUT_PREFIX "Warning: Empty IP\n");
            return;
        }
        RequestSource src;
        src.ip = ip;

        std::string srcId = src.getUniqueIdentifier();
        auto itr = requestTimeWindows.find(srcId);
        
        if(itr == requestTimeWindows.end()) {
            requestTimeWindows[srcId] = std::list<time_t>();
            itr = requestTimeWindows.find(srcId);
        }

        std::list<time_t>& rtw = itr -> second;

        time_t currentTime = time(0);

        rtw.push_back(currentTime);

        if(rtw.size() < REQUEST_TIME_WINDOW_SIZE) return;

        while(rtw.size() > REQUEST_TIME_WINDOW_SIZE) {
            rtw.erase(rtw.begin());
        }

        if(currentTime - *rtw.begin() < MIN_TIME_DIFF_PER_WINDOW) {
            rules[src.getUniqueIdentifier()] = RequestSourceRule(src, false); // Ban the request source if request frequency too high
        }
    }

    void inspectRequestInfo(RequestInfo& info) {
        auto itr = info.find("ip");
        if(itr != info.end()) {
            inspectRequestIp(itr -> second);
        } else {
            fprintf(stderr, CONSOLE_OUTPUT_PREFIX "Warning: No IP provided\n");
        }
    }

    void removeExpiredRules() {
        time_t currentTime = time(0);

        for(auto itr = rules.begin(); itr != rules.end();) {
            if(currentTime - itr -> second.createTime > RULE_EXPIRE_TIME) {
                auto prevItr = itr;
                itr++;
                rules.erase(prevItr);
            } else {
                itr++;
            }
        }
    }

    void removeExpiredRequests() {
        time_t currentTime = time(0);

        for(auto itr = requestTimeWindows.begin(); itr != requestTimeWindows.end();) {
            if(
                itr -> second.empty()
                || currentTime - *(itr -> second.rbegin()) > REQUEST_EXPIRE_TIME
            ) {
                auto prevItr = itr;
                itr++;
                requestTimeWindows.erase(prevItr);
            } else {
                itr++;
            }
        }
    }

    void inspectionWorker() {
        while(1) {
            if(!workerNotification) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue; // Wait for notification.
            }

            workerNotification = false;

            workerLock.lock();

            auto& requestInfoQueue = *requestInfoQueuePtr;
            requestInfoQueuePtr = new std::list<RequestInfo>();

            workerLock.unlock();

            for(auto itr = requestInfoQueue.begin(); itr != requestInfoQueue.end(); itr++) {
                inspectRequestInfo(*itr);
            }
            removeExpiredRules();
            removeExpiredRequests();

            delete &requestInfoQueue;
        }
    }

    void startInspectionThread() {
        if(inspectionThread) return;

        requestInfoQueuePtr = new std::list<RequestInfo>();

        inspectionThread = new std::thread(inspectionWorker);
    }
}
