//
// Created on 2025/8/22.
// panyi siwangqishiq
// 
// please 

#pragma once
#include "my_account.h"
#include "my_call.h"
#ifndef HARMOYPJSIPDEMO_SIP_APP_H
#define HARMOYPJSIPDEMO_SIP_APP_H

#include "napi/native_api.h"
#include <string>
#include "pjsua2.hpp"
#include <memory>
#include "napi/native_api.h"
#include <vector>

const std::string SIP_PROTOCOL = "sip:";
const std::string SIP_SERVER = "192.168.101.236:5060";

const std::string OBSERVER_METHOD_REGSTATE_CHANGE = "onRegStateChanged";
const std::string OBSERVER_METHOD_INCOMING_CALL = "onIncomingCall";
const std::string OBSERVER_METHOD_START_AUDIO = "onStartAudio";
const std::string OBSERVER_METHOD_DISCONNECT = "onDisconnect";

struct SipAppJsParams{
    SipApp* appContext;
    std::string method;
    int code;
    std::string params;
};

class MyCall;

class SipApp {
public:
    static const std::string TAG;
    
    SipApp();

    void initEnv(napi_env env);
    
    void registerObserver(napi_value observer);
    void UnRegisterObserver(napi_value observer);
    
    void sipLogin(std::string account, std::string password);
    
    virtual ~SipApp();
    
    std::unique_ptr<pj::Endpoint>& getEndpoint(){
        return endpoint_;
    }
    
    bool checkObserverListHaveValue(napi_value ob);
    
    std::vector<napi_ref>& getObserverList(){
        return observerRefList;
    }
    
    void fireObserverCallback(std::string methodName,int what,std::string params);
    
    void jsMethodRouter(SipAppJsParams *jsParams);
    
    std::vector<std::shared_ptr<MyCall>>& getCallList(){
        return callList;
    }
    
    static void invokeJs(napi_env env, napi_value js_cb, void* context, void* data);
    
    void hangup(std::string &callId, bool isBusy);
    void accept(std::string &callId);
    
    bool removeCall(std::string &callId);
private:
    napi_env g_env;
    napi_threadsafe_function g_tsfn;
    
    std::vector<napi_ref> observerRefList;
    
    std::unique_ptr<pj::Endpoint> endpoint_;
    std::shared_ptr<MyAccount> account_ = nullptr;
    
    std::vector<std::shared_ptr<MyCall>> callList;
    
    std::shared_ptr<MyCall> findCallByCallId(std::string &callId);
};

#endif //HARMOYPJSIPDEMO_SIP_APP_H
