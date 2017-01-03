#include <node.h>

#include <string>
#include <map>

#include "Inspection.h"

using namespace v8;

static void requestInfoInput(const FunctionCallbackInfo<Value>& info) {
    Isolate *isolate = info.GetIsolate();

    if(
        info.Length() != 1
        || !info[0] -> IsObject()
    ) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Invalid arguments"));
        return;
    }

    std::map<std::string, std::string> requestProperties;

    Local<Context> currentContext = isolate -> GetCurrentContext();
    Local<Object> requestInfo = info[0] -> ToObject(currentContext).ToLocalChecked();
    Local<Array> propNames = requestInfo -> GetOwnPropertyNames(currentContext).ToLocalChecked();

    for(int i = 0, l = propNames -> Length(); i < l; i++) {
        Local<Value> name = propNames -> Get(i);
        Local<Value> value = requestInfo -> Get(currentContext, name).ToLocalChecked();

        std::string nameStr = *String::Utf8Value(name);
        std::string valueStr = *String::Utf8Value(value);

        requestProperties[nameStr] = valueStr;
    }

    Inspection::addRequestInfoToQueue(requestProperties);
}

static void onCheckRequest(const FunctionCallbackInfo<Value>& info) {
    Isolate *isolate = info.GetIsolate();

    if(
        info.Length() != 1
        || !info[0] -> IsObject()
    ) {
        isolate -> ThrowException(String::NewFromUtf8(isolate, "Invalid arguments"));
        return;
    }

    std::map<std::string, std::string> requestProperties;

    Local<Context> currentContext = isolate -> GetCurrentContext();
    Local<Object> requestInfo = info[0] -> ToObject(currentContext).ToLocalChecked();
    Local<Array> propNames = requestInfo -> GetOwnPropertyNames(currentContext).ToLocalChecked();

    for(int i = 0, l = propNames -> Length(); i < l; i++) {
        Local<Value> name = propNames -> Get(i);
        Local<Value> value = requestInfo -> Get(currentContext, name).ToLocalChecked();

        std::string nameStr = *String::Utf8Value(name);
        std::string valueStr = *String::Utf8Value(value);

        requestProperties[nameStr] = valueStr;
    }

    info.GetReturnValue().Set(Inspection::checkRequest(requestProperties));
}

static void moduleInit(Local<Object> exports) {
    Inspection::startInspectionThread();
    NODE_SET_METHOD(exports, "requestInfoInput", requestInfoInput);
    NODE_SET_METHOD(exports, "checkRequest", onCheckRequest);
}

NODE_MODULE(Core, moduleInit)
