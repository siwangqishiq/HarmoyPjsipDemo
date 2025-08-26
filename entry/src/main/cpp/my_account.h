//
// Created on 2025/8/25.
// panyi

#pragma once

#ifndef HARMOYPJSIPDEMO_MY_ACCOUNT_H
#define HARMOYPJSIPDEMO_MY_ACCOUNT_H

#include "pjsua2.hpp"
#include <memory>

class SipApp;
class MyAccount : public pj::Account {
public:
    MyAccount(SipApp *ctx);
    virtual ~MyAccount();
    
    virtual void create(std::string account, std::string password);
    
    virtual void onRegState(pj::OnRegStateParam &prm) override;
    
    virtual void onIncomingCall(pj::OnIncomingCallParam &iprm) override;
private:
    SipApp *appContext = nullptr;
};

#endif //HARMOYPJSIPDEMO_MY_ACCOUNT_H
