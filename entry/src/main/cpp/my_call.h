//
// Created on 2025/8/25.
// panyi

#ifndef HARMOYPJSIPDEMO_MY_CALL_H
#define HARMOYPJSIPDEMO_MY_CALL_H

#include "pjsua2.hpp"
#include "my_account.h"

class SipApp;
class MyCall : public pj::Call {
public:
    MyCall(SipApp *ctx,
           MyAccount &acc, 
           int call_id = PJSUA_INVALID_ID): Call(acc, call_id){
        this->appContext = ctx;
    }
    
    ~MyCall(){}

    virtual void onCallState(pj::OnCallStateParam &prm) override;

    virtual void onCallMediaState(pj::OnCallMediaStateParam &prm) override;
    
    void setCallId(std::string callId){
        this->callId = callId;
    }
    
    std::string getCallId() const{
        return this->callId;
    }
    
    bool isDisconnect();
private:
    SipApp *appContext = nullptr;
    std::string callId;
};

#endif //HARMOYPJSIPDEMO_MY_CALL_H
