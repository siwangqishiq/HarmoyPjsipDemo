//
// Created on 2025/8/22.
// panyi siwangqishiq
// 
// please 

#pragma once
#include "my_account.h"
#ifndef HARMOYPJSIPDEMO_SIP_APP_H
#define HARMOYPJSIPDEMO_SIP_APP_H

#include "napi/native_api.h"
#include <string>
#include "pjsua2.hpp"
#include <memory>

const std::string SIP_PROTOCOL = "sip:";
const std::string SIP_SERVER = "192.168.102.72:5060";

class SipApp {
public:
    static const std::string TAG;
    
    SipApp();
    
    void sipLogin(std::string account, std::string password);
    
    virtual ~SipApp();
    
    std::unique_ptr<pj::Endpoint>& getEndpoint(){
        return endpoint_;
    }
private:
    std::unique_ptr<pj::Endpoint> endpoint_;
    std::shared_ptr<MyAccount> account_ = nullptr;
};

#endif //HARMOYPJSIPDEMO_SIP_APP_H
