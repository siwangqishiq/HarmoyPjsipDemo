//
// Created on 2025/8/25.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "my_account.h"
#include "log.h"
#include "sip_app.h"
#include <thread>

MyAccount::MyAccount(){
    NLOGI("MyAccount construct");
}

MyAccount::~MyAccount(){
    NLOGI("MyAccount deConstruct~");
    Account::shutdown();
}

void MyAccount::create(std::string account, std::string password){
    std::string accId = SIP_PROTOCOL + account +"@" + SIP_SERVER;
    
    pj::AccountConfig accountConfig;
    accountConfig.idUri = accId;
    accountConfig.regConfig.registrarUri = SIP_PROTOCOL + SIP_SERVER +";transport=tcp";
    
    pj::AuthCredInfo authInfo;
    authInfo.dataType = PJSIP_CRED_DATA_PLAIN_PASSWD;
    authInfo.scheme = "digest";
    authInfo.username = account;
    authInfo.data = password;
    authInfo.realm = "*";
    accountConfig.regConfig.timeoutSec = 60;
    
    accountConfig.sipConfig.authCreds.push_back(std::move(authInfo));
    Account::create(accountConfig);
}
    
void MyAccount::onRegState(pj::OnRegStateParam &prm) {
    NLOGI("MyAccount on reg state changed code = %{public}d" , prm.code);
    NLOGI("MyAccount on reg state changed reason = %{public}s" , prm.reason.c_str());
    NLOGI("MyAccount onRegState thread id : %{public}d", std::this_thread::get_id());
}
    
void MyAccount::onIncomingCall(pj::OnIncomingCallParam &iprm){
    NLOGI("MyAccount onIncomingCall callid:%d" , iprm.callId);
}