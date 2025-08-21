//
// Created on 2025/8/21.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef HARMOYPJSIPDEMO_LOG_H
#define HARMOYPJSIPDEMO_LOG_H

#include <hilog/log.h>

#define LOG_TAG "native"

#define NLOGI(fmt, ...) OH_LOG_Print(LOG_APP, LOG_INFO,  LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define NLOGE(fmt, ...) OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)
#define NLOGD(fmt, ...) OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, fmt, ##__VA_ARGS__)

//class Log {
//public:
//    static void i(std::string tag, const char *fmt, ...);
////    static void w(std::string tag, std::string msg);
////    static void e(std::string tag, std::string msg);
//};

#endif //HARMOYPJSIPDEMO_LOG_H
