//
// Created on 2025/8/25.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "my_account.h"
#include "log.h"
#include "sip_app.h"
#include <thread>
#include "my_call.h"

MyAccount::MyAccount(SipApp *ctx){
    NLOGI("MyAccount construct");
    appContext = ctx;
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

    // accountConfig.natConfig.iceEnabled = true;
    // accountConfig.callConfig.prackUse = PJSUA_100REL_NOT_USED;
    
    accountConfig.sipConfig.authCreds.push_back(std::move(authInfo));
    Account::create(accountConfig);
}
    
void MyAccount::onRegState(pj::OnRegStateParam &prm) {
    const int code = prm.code;
    std::string msg = prm.reason;
    
    NLOGI("MyAccount on reg state changed code = %{public}d" , prm.code);
    NLOGI("MyAccount on reg state changed reason = %{public}s" , prm.reason.c_str());
    NLOGI("MyAccount onRegState thread id : %{public}d", std::this_thread::get_id());
    
    appContext->fireObserverCallback(OBSERVER_METHOD_REGSTATE_CHANGE, code, msg);
}
    
void MyAccount::onIncomingCall(pj::OnIncomingCallParam &iprm){
    int callId = iprm.callId;
    std::string wholeMsg = iprm.rdata.wholeMsg;
    NLOGI("MyAccount onIncomingCall callid:%{public}d" , callId);
    NLOGI("MyAccount onIncomingCall wholeMsg:%{public}s" , wholeMsg.c_str());
    
    try{
        std::shared_ptr<MyCall> call = std::make_shared<MyCall>(this->appContext, *this, iprm.callId);
        call->setCallId(iprm.rdata.info);
        
        pj::CallOpParam opParam;
        opParam.statusCode = PJSIP_SC_RINGING;
        call->answer(opParam);
        this->appContext->getCallList().emplace_back(call);
        
        //callback
        this->appContext->fireObserverCallback(OBSERVER_METHOD_INCOMING_CALL, 0, call->getCallId());
    } catch (std::exception &e) {
        NLOGE("onIncomingCall error : %{public}s", e.what());
    }
}

void MyAccount::onInstantMessage(pj::OnInstantMessageParam &prm){
    NLOGI("MyAccount onInstantMessage from:%{public}s to:%{public}s" , 
          prm.fromUri.c_str(), prm.toUri.c_str());
    NLOGI("MyAccount onInstantMessage message:%{public}s" , prm.msgBody.c_str());
}