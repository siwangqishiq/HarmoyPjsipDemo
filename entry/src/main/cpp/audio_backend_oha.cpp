//
// Created panyi
// 
//

#include "audio_backend_oha.h"
#include "log.h"
#include <ohaudio/native_audiocapturer.h>
#include <ohaudio/native_audiorenderer.h>
#include <ohaudio/native_audiostreambuilder.h>

typedef struct{
    pjmedia_aud_dev_factory  base;
    pj_pool_factory* pf;
    pj_pool_t* pool;
    
    OH_AudioStreamBuilder* builder;
    OH_AudioRenderer* renderer;
    OH_AudioCapturer* capture;
    
    bool isRenderAudio;
    bool isCaptureAudio;
} ohaudio_factory;

pj_status_t OhaudioInit(pjmedia_aud_dev_factory *f){
    NLOGI("OhaudioInit");
//    ohaudio_factory *ohFactory = reinterpret_cast<ohaudio_factory *>(f);
    return PJ_SUCCESS;
}

pj_status_t OhaudioDestroy(pjmedia_aud_dev_factory *f){
    ohaudio_factory *ohFactory = reinterpret_cast<ohaudio_factory *>(f);
    
    return PJ_SUCCESS;
}

unsigned OhaudioDevCount(pjmedia_aud_dev_factory *f){
    return 1;
}

pj_status_t OhaudioDevInfo(pjmedia_aud_dev_factory *f,unsigned index,pjmedia_aud_dev_info *info){
    
    return PJ_SUCCESS;
}

pj_status_t OhaudioDefaultParam(pjmedia_aud_dev_factory *f,unsigned index,pjmedia_aud_param *param){
    return PJ_SUCCESS;
}

pj_status_t OhaudioCreateStream(pjmedia_aud_dev_factory *f,const pjmedia_aud_param *param,
                                 pjmedia_aud_rec_cb rec_cb,
                                 pjmedia_aud_play_cb play_cb,
                                 void *user_data,pjmedia_aud_stream **p_aud_strm){
    return PJ_SUCCESS;    
}

pj_status_t OhaudioRefresh(pjmedia_aud_dev_factory *f){
    return PJ_SUCCESS;
}


static pjmedia_aud_dev_factory_op ohaudio_factory_operations = {
    &OhaudioInit,
    &OhaudioDestroy,
    &OhaudioDevCount,
    &OhaudioDevInfo,
    &OhaudioDefaultParam,
    &OhaudioCreateStream,
    &OhaudioRefresh
};

pjmedia_aud_dev_factory* AudioBackendOha::buildFactory(pj_pool_factory *pf){
    NLOGI("AudioBackend OHA build factory");
    
    ohaudio_factory *f;
    pj_pool_t *pool;
    
    pool = pj_pool_create(pf, "oh_audio", 1024, 1024, nullptr);
    f = PJ_POOL_ZALLOC_T(pool, ohaudio_factory);
    f->pf = pf;
    f->pool = pool;
    f->base.op = &ohaudio_factory_operations;
    
    f->builder = nullptr;
    f->renderer = nullptr;
    f->capture = nullptr;
    f->isRenderAudio = false;
    f->isCaptureAudio = false;
    
    return &f->base;
}

