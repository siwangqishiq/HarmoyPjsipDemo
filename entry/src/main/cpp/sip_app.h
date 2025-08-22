//
// Created on 2025/8/22.
// panyi siwangqishiq
// 
// please 

#pragma once
#ifndef HARMOYPJSIPDEMO_SIP_APP_H
#define HARMOYPJSIPDEMO_SIP_APP_H

#include "napi/native_api.h"
#include <string>
#include "pjsua2.hpp"
#include <memory>


class SipApp {
public:
    static const std::string TAG;
    
    SipApp();
    
    virtual ~SipApp();
    
    std::unique_ptr<pj::Endpoint>& getEndpoint(){
        return endpoint_;
    }
private:
    std::unique_ptr<pj::Endpoint> endpoint_;
};

#endif //HARMOYPJSIPDEMO_SIP_APP_H
