#include "napi/native_api.h"
#include <string>

#include "pjsua2.hpp"
extern "C" {
#include "pjlib.h"
}
#include "log.h"

static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args , nullptr, nullptr);

    napi_valuetype valuetype0;
    napi_typeof(env, args[0], &valuetype0);

    napi_valuetype valuetype1;
    napi_typeof(env, args[1], &valuetype1);

    double value0;
    napi_get_value_double(env, args[0], &value0);

    double value1;
    napi_get_value_double(env, args[1], &value1);

    napi_value sum;
    napi_create_double(env, value0 + value1, &sum);
    
    return sum;
}

static napi_value NAPI_Global_getNativeString(napi_env env, napi_callback_info info) {
    napi_value str;
    std::string native_str = "你好世界";
    napi_create_string_utf8(env, native_str.c_str(), native_str.length(), &str);
    const char *test = "hello";
//    Log::i("native", "1get native string = %{public}s",test);
    NLOGI("1get native string = %{public}s",native_str.c_str());
    NLOGI("1get native string = 1 + 2 = %{public}i",1+2);
//    Log::i("native", "get native string end");
    return str;
}

static napi_value NAPI_Global_getPjsipVersionStr(napi_env env, napi_callback_info info) {
    pj::Endpoint ep;
    ep.libCreate();
    
    pj::EpConfig ep_cfg;
    ep.libInit(ep_cfg);
    std::string pjVersion = pj_get_version();
    ep.libDestroy();
    
    napi_value result;
    napi_create_string_utf8(env, pjVersion.c_str(), pjVersion.length(), &result);
    return result;
}


EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"add", nullptr, Add, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getNativeString", nullptr, NAPI_Global_getNativeString, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getPjsipVersionStr", nullptr, NAPI_Global_getPjsipVersionStr, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
    napi_module_register(&demoModule);
}
