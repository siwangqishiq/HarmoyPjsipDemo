//
// Created on 2025/8/22.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "sip_app.h"
#include "log.h"
#include "my_account.h"

const std::string SipApp::TAG = "sip_app";

class MyLogger : public pj::LogWriter{
public:
    virtual void write(const pj::LogEntry &entry) override {
        NLOGI("pjsip:%{public}s", entry.msg.c_str());
    }
};

SipApp::SipApp(){
    NLOGI("SipApp construct");
    endpoint_ = std::make_unique<pj::Endpoint>();
    try{
        endpoint_->libCreate();
        
        pj::EpConfig config;
        config.logConfig.level = 5;
        config.uaConfig.maxCalls = 4;
        config.uaConfig.userAgent = "panyi_ua";
        
        config.logConfig.level = 5;          // 日志等级，范围0-5
        config.logConfig.consoleLevel = 4;   // 控制台打印等级
        config.logConfig.writer = new MyLogger(); // 自定义日志写入器
        
        endpoint_->libInit(config);
        NLOGI("SipApp endpoint lib init");
        
        pj::TransportConfig transConfig;
        transConfig.port = 5060;
        auto transId = endpoint_->transportCreate(PJSIP_TRANSPORT_TCP, transConfig);
        NLOGI("SipApp transportCreate ID = %{public}d", transId);
        endpoint_->libStart();
        NLOGI("SipApp endpoint lib started");
        
//        NLOGI("SipApp endpoint setNullDev");
//        endpoint_->audDevManager().setNullDev();
        
        pj::AudioDevInfoVector2 devList = endpoint_->audDevManager().enumDev2();
        NLOGI("deviceList size = %{public}d", devList.size());
        for(auto &dev : devList){
            NLOGI("dev name : %{public}s", dev.name.c_str());
        }
        
//        pj::AudDevManager &audioMgr = endpoint_->audDevManager();
//        int capDev = audioMgr.getCaptureDev();
//        int playDev = audioMgr.getPlaybackDev();
//        NLOGI("Before set CaptureDev=%{public}d, PlaybackDev=%{public}d", capDev, playDev);
//        for(int i = 0 ; i < devList.size();i++){
//            audioMgr.setCaptureDev(i);
//            audioMgr.setPlaybackDev(i);
//        }//end for i
//        capDev = audioMgr.getCaptureDev();
//        playDev = audioMgr.getPlaybackDev();
//        NLOGI("After set CaptureDev=%{public}d, PlaybackDev=%{public}d", capDev, playDev);
    } catch (std::exception &e) {
        NLOGE("SipApp endpoint lib create error %s", e.what());
    }
}

void SipApp::initEnv(napi_env env){
    this->g_env = env;

    napi_value resource_name;
    napi_create_string_utf8(env, "SipObserverTsfn", NAPI_AUTO_LENGTH, &resource_name);

    napi_status result = napi_create_threadsafe_function(env, 
       nullptr, 
       nullptr, 
       resource_name, 
       0, 
       1, 
       nullptr, 
       nullptr, 
       this, 
       invokeJs, 
       &g_tsfn);
    
    if(result == napi_ok){
        NLOGI("create tsfn success");
    }else{
        NLOGE("create tsfn failed.");
    }//end if
}
    
SipApp::~SipApp(){
    endpoint_->libDestroy();
    NLOGI("SipApp disposed");
}

void SipApp::sipLogin(std::string account, std::string password){
    NLOGI("SipApp sipLogin");
    
    account_ = std::make_shared<MyAccount>(this);
    account_->create(account, password);
    
    NLOGI("sipLogin thread id : %{public}d", std::this_thread::get_id());
}

bool SipApp::checkObserverListHaveValue(napi_value ob){
    if(observerRefList.empty()){
        return false;
    }
    
    for(auto it = observerRefList.begin();it != observerRefList.end(); ++it){
        napi_value refObj;
        napi_get_reference_value(g_env, *it, &refObj);
        bool isEqual;
        napi_strict_equals(g_env, refObj, ob, &isEqual);
        if(isEqual){
            return true;
        }
    }//end for iter
    return false;
}

void SipApp::registerObserver(napi_value observer){
    if(checkObserverListHaveValue(observer)){
        NLOGE("observer have already in list");
        return;
    }
    
    napi_ref ref;
    auto result = napi_create_reference(g_env, observer, 1, &ref);
    if(result != napi_ok){
        NLOGE("create ref error");
        return;       
    }
    observerRefList.push_back(ref);
}

void SipApp::UnRegisterObserver(napi_value observer){
    if(!checkObserverListHaveValue(observer)){
        NLOGE("observer not in list");
        return;
    }
    
    for (auto it = observerRefList.begin(); it != observerRefList.end(); ++it) {
        napi_value refObj;
        napi_get_reference_value(g_env, *it, &refObj);

        bool isEqual;
        napi_strict_equals(g_env, observer, refObj, &isEqual);

        if (isEqual) {
            napi_delete_reference(g_env, *it);
            observerRefList.erase(it);
            break;
        }
    }//end for each
}

void SipApp::fireObserverCallback(std::string methodName,int what,std::string params){
    NLOGI("fireObserverCallback");
    if(observerRefList.empty()){
        return;
    }
    
    SipAppJsParams *jsParams = new SipAppJsParams();
    jsParams->method = methodName;
    jsParams->code = what;
    jsParams->params = params;
    
    napi_call_threadsafe_function(g_tsfn, jsParams, napi_tsfn_nonblocking);
}

void SipApp::jsMethodRouter(SipAppJsParams *jsParams){
    NLOGI("SipApp jsMethodRouter");
    if(jsParams == nullptr){
        return;
    }
    
    NLOGI("SipApp jsMethodRouter method %{public}s", jsParams->method.c_str());
    NLOGI("jsMethodRouter thread id : %{public}d", std::this_thread::get_id());
    
    std::vector<napi_ref> listCopy = this->observerRefList;
    if(listCopy.empty()){
        return;
    }
    
    for(napi_ref &ref : listCopy){
        napi_value ob;
        if(napi_get_reference_value(g_env, ref, &ob) != napi_ok){
            continue;
        }
        
        napi_value jsFunc;
        if(napi_get_named_property(g_env, ob, jsParams->method.c_str(), &jsFunc)!= napi_ok){
            continue;
        }
        
        if(OBSERVER_METHOD_REGSTATE_CHANGE == jsParams->method){
            const int paramsCount = 2;
            napi_value argv[paramsCount];
            napi_create_int32(g_env, jsParams->code, &argv[0]);
            napi_value jsMsg;
            napi_create_string_utf8(g_env, jsParams->params.c_str(), NAPI_AUTO_LENGTH, &jsMsg);
            argv[1] = jsMsg;
            napi_call_function(g_env, ob, jsFunc, paramsCount, argv, nullptr);
        }else{
            const int paramsCount = 1;
            napi_value argv[paramsCount];
            napi_create_string_utf8(g_env, jsParams->params.c_str(), NAPI_AUTO_LENGTH, argv);
            napi_call_function(g_env, ob, jsFunc, paramsCount, argv, nullptr);
        }//end if
    }//end for each
}

void SipApp::invokeJs(napi_env env, napi_value js_cb, void* context, void* data){
    NLOGI("invokeJs thread id : %{public}d", std::this_thread::get_id());
    
    SipApp *app = static_cast<SipApp *>(context);
    SipAppJsParams* jsParams = static_cast<SipAppJsParams *>(data);
    app->jsMethodRouter(jsParams);
    
    if(jsParams){
        delete jsParams;
    }
}

std::shared_ptr<MyCall> SipApp::findCallByCallId(std::string &callId){
    std::shared_ptr<MyCall> result = nullptr;
    for(std::shared_ptr<MyCall> &ptr : callList){
        if(ptr->getCallId() == callId){
            result = ptr;
            break;
        }
    }//end for each
    return result;
}

void SipApp::hangup(std::string &callId,bool isBusy){
    auto call = findCallByCallId(callId);
    if(call == nullptr){
        return;
    }
    
    pj::CallOpParam param;
    if(isBusy){
        param.statusCode = PJSIP_SC_BUSY_HERE;
    }else{
        param.statusCode = PJSIP_SC_DECLINE;
    }
    call->hangup(param);
}

void SipApp::accept(std::string &callId){
    auto call = findCallByCallId(callId);
    if(call == nullptr){
        return;
    }
    
    pj::CallOpParam param;
    param.statusCode = PJSIP_SC_OK;
    call->answer(param);
}

bool SipApp::removeCall(std::string &callId){
    for(auto it = callList.begin();it != callList.end();++it){
        if(callId == (*it)->getCallId()){
            callList.erase(it);
            return true;        
        }
    }//end for each
    return false;
}

