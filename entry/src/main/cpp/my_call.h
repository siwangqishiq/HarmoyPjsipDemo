//
// Created on 2025/8/25.
// panyi

#ifndef HARMOYPJSIPDEMO_MY_CALL_H
#define HARMOYPJSIPDEMO_MY_CALL_H

#include "pjsua2.hpp"
#include "my_account.h"

class MyCall : public pj::Call {
public:
    MyCall(MyAccount &acc, int call_id = PJSUA_INVALID_ID): Call(acc, call_id){
    }

    ~MyCall(){}

    virtual void onCallState(pj::OnCallStateParam &prm) override;

    virtual void onCallMediaState(pj::OnCallMediaStateParam &prm) override;
};

#endif //HARMOYPJSIPDEMO_MY_CALL_H
