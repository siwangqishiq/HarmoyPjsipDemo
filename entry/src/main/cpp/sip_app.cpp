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

