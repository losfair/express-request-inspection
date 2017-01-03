#include <map>
#include <list>
#include <string>

#include <time.h>

namespace Inspection {
    typedef std::map<std::string, std::string> RequestInfo;

    class RequestSource {
        public:
            std::string ip;
            std::string getUniqueIdentifier();
    };

    class RequestSourceRule {
        public:
            RequestSource src;
            bool isAllowed;
            time_t createTime;

            RequestSourceRule();

            RequestSourceRule(RequestSource _src, bool _isAllowed);
            RequestSourceRule(std::string srcIp, bool _isAllowed);
            std::string getUniqueIdentifier();
    };

    bool checkRequest(RequestInfo& info);
    void addRequestInfoToQueue(RequestInfo& reqInfo);
    void startInspectionThread();
}