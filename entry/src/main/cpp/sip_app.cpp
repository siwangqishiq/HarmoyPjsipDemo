//
// Created on 2025/8/22.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "sip_app.h"
#include "log.h"
#include "my_account.h"

const std::string SipApp::TAG = "sip_app";

SipApp::SipApp(){
    NLOGI("SipApp construct");
    endpoint_ = std::make_unique<pj::Endpoint>();
    try{
        endpoint_->libCreate();
        
        pj::EpConfig config;
        config.logConfig.level = 5;
        config.uaConfig.maxCalls = 4;
        config.uaConfig.userAgent = "panyi_ua";
        
        endpoint_->libInit(config);
        NLOGI("SipApp endpoint lib init");
        
        pj::TransportConfig transConfig;
        transConfig.port = 5060;
        auto transId = endpoint_->transportCreate(PJSIP_TRANSPORT_TCP, transConfig);
        NLOGI("SipApp transportCreate ID = %{public}d", transId);
        
        endpoint_->libStart();
        NLOGI("SipApp endpoint lib started");
    } catch (std::exception &e) {
        NLOGE("SipApp endpoint lib create error %s", e.what());
    }
}

void SipApp::initEnv(napi_env env){
    this->g_env = env;
//    napi_create_threadsafe_function(env, 
//        napi_value func, 
//        nullptr, 
//        napi_value async_resource_name, 
//        0, 
//        1, 
//        nullptr, 
//        nullptr, 
//        nullptr, 
//        napi_threadsafe_function_call_js call_js_cb, 
//        &tsfn);
}
    
SipApp::~SipApp(){
    endpoint_->libDestroy();
    NLOGI("SipApp disposed");
}

void SipApp::sipLogin(std::string account, std::string password){
    NLOGI("SipApp sipLogin");
    
    account_ = std::make_shared<MyAccount>();
    account_->create(account, password);
    
    NLOGI("sipLogin thread id : %{public}d", std::this_thread::get_id());
}

bool SipApp::checkObserverListHaveValue(napi_value ob){
    if(observerRefList.empty()){
        return false;
    }
    
    for(auto it = observerRefList.begin();it != observerRefList.end(); ++it){
        napi_value refObj;
        napi_get_reference_value(g_env, *it, &refObj);
        bool isEqual;
        napi_strict_equals(g_env, refObj, ob, &isEqual);
        if(isEqual){
            return true;
        }
    }//end for iter
    return false;
}

void SipApp::registerObserver(napi_value observer){
    if(checkObserverListHaveValue(observer)){
        NLOGE("observer have already in list");
        return;
    }
    
    napi_ref ref;
    auto result = napi_create_reference(g_env, observer, 1, &ref);
    if(result != napi_ok){
        NLOGE("create ref error");
        return;       
    }
    observerRefList.push_back(ref);
}

void SipApp::UnRegisterObserver(napi_value observer){
    if(!checkObserverListHaveValue(observer)){
        NLOGE("observer not in list");
        return;
    }
    
    for (auto it = observerRefList.begin(); it != observerRefList.end(); ++it) {
        napi_value refObj;
        napi_get_reference_value(g_env, *it, &refObj);

        bool isEqual;
        napi_strict_equals(g_env, observer, refObj, &isEqual);

        if (isEqual) {
            napi_delete_reference(g_env, *it);
            observerRefList.erase(it);
            break;
        }
    }//end for each
}

