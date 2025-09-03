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

typedef struct{
    pjmedia_aud_stream base;
    pj_pool_t* pool;
    pj_str_t name;
    pjmedia_dir dir;
    pjmedia_aud_param param;
    
    void *user_data;
    bool isQuit;
    pjmedia_aud_rec_cb recCallback;
    pjmedia_aud_play_cb playCallback;

    OH_AudioRenderer* renderer;
    OH_AudioCapturer* capture;
} ohaudio_stream;

pj_status_t OhStreamGetParam(pjmedia_aud_stream *s,pjmedia_aud_param *param){
    NLOGI("OhStreamGetParam");
    ohaudio_stream *strm = reinterpret_cast<ohaudio_stream *>(s);
    if(strm == nullptr || param == nullptr){
        return PJ_EINVAL;
    }
    pj_memcpy(param, &strm->param, sizeof(*param));
    return PJ_SUCCESS;
}

pj_status_t OhStreamGetCap(pjmedia_aud_stream *s,pjmedia_aud_dev_cap cap,void *value){
    return PJ_SUCCESS;
}

pj_status_t OhStreamSetCap(pjmedia_aud_stream *s,pjmedia_aud_dev_cap cap,const void *value){
    return PJ_SUCCESS;
}

pj_status_t OhStreamStart(pjmedia_aud_stream *s){
    NLOGI("OhStreamStart");
    
    ohaudio_stream *strm = reinterpret_cast<ohaudio_stream *>(s);
    strm->isQuit = false;
    
    if(strm->renderer != nullptr){
        OH_AudioRenderer_Start(strm->renderer);
    }
    
    if(strm->capture != nullptr){
        OH_AudioCapturer_Start(strm->capture);
    }
    
    return PJ_SUCCESS;
}

pj_status_t OhStreamStop(pjmedia_aud_stream *s){
    NLOGI("OhStreamStop");
    
    ohaudio_stream *strm = reinterpret_cast<ohaudio_stream *>(s);
    strm->isQuit = true;
    
    if(strm->renderer != nullptr){
        OH_AudioRenderer_Stop(strm->renderer);
    }
    
    if(strm->capture != nullptr){
        OH_AudioCapturer_Stop(strm->capture);
    }
    return PJ_SUCCESS;
}

pj_status_t OhStreamDestroy(pjmedia_aud_stream *s){
    NLOGI("OhStreamDestroy");
    OhStreamStop(s);
    ohaudio_stream *strm = reinterpret_cast<ohaudio_stream *>(s);
    
    if(strm->renderer != nullptr){
        OH_AudioRenderer_Flush(strm->renderer);
        OH_AudioRenderer_Release(strm->renderer);
    }
    
    if(strm->capture != nullptr){
        OH_AudioCapturer_Flush(strm->capture);
        OH_AudioCapturer_Release(strm->capture);
    }
    
    pj_pool_release(strm->pool);
    return PJ_SUCCESS;
}

static pjmedia_aud_stream_op stream_operations = {
    &OhStreamGetParam,
    &OhStreamGetCap,
    &OhStreamSetCap,
    &OhStreamStart,
    &OhStreamStop,
    &OhStreamDestroy
};

pj_status_t OhaudioInit(pjmedia_aud_dev_factory *f){
    NLOGI("OhaudioInit");
    return PJ_SUCCESS;
}

pj_status_t OhaudioDestroy(pjmedia_aud_dev_factory *f){
    NLOGI("OhaudioDestroy");
    ohaudio_factory *ohFactory = reinterpret_cast<ohaudio_factory *>(f);
    if(ohFactory->renderer != nullptr){
        OH_AudioRenderer_Stop(ohFactory->renderer);
        ohFactory->renderer = nullptr;
        ohFactory->isRenderAudio = false;
    }
    
    if(ohFactory->capture != nullptr){
        OH_AudioCapturer_Stop(ohFactory->capture);
        ohFactory->capture = nullptr;
        ohFactory->isCaptureAudio = false;
    }
    
    if(ohFactory->builder != nullptr){
        OH_AudioStreamBuilder_Destroy(ohFactory->builder);
        ohFactory->builder = nullptr;   
    }
    
    pj_pool_t* pool = ohFactory->pool;
    ohFactory->pool = nullptr;
    pj_pool_release(pool);
    return PJ_SUCCESS;
}

unsigned OhaudioDevCount(pjmedia_aud_dev_factory *f){
    return 1;
}

pj_status_t OhaudioDevInfo(pjmedia_aud_dev_factory *f,unsigned index,pjmedia_aud_dev_info *info){
    ohaudio_factory *ohFactory = reinterpret_cast<ohaudio_factory *>(f);
    
    pj_bzero(info, sizeof(*info));
    strcpy(info->name, "OH_Audio_backend");
    strcpy(info->driver, "siwangqishiq");
    info->default_samples_per_sec = 8000;
    info->input_count = 1;
    info->output_count = 1;
    
    return PJ_SUCCESS;
}

pj_status_t OhaudioDefaultParam(pjmedia_aud_dev_factory *f,unsigned index,pjmedia_aud_param *param){
//    ohaudio_factory *ohFactory = reinterpret_cast<ohaudio_factory *>(f);
    pjmedia_aud_dev_info adi;
    pj_status_t status;
    
    status = OhaudioDevInfo(f, index, &adi);
    if (status != PJ_SUCCESS)
        return status;
    
    pj_bzero(param, sizeof(*param));
    if (adi.input_count && adi.output_count) {
        param->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
        param->rec_id = index;
        param->play_id = index;
    } else if (adi.input_count) {
        param->dir = PJMEDIA_DIR_CAPTURE;
        param->rec_id = index;
        param->play_id = PJMEDIA_AUD_INVALID_DEV;
    } else if (adi.output_count) {
        param->dir = PJMEDIA_DIR_PLAYBACK;
        param->play_id = index;
        param->rec_id = PJMEDIA_AUD_INVALID_DEV;
    } else {
        return PJMEDIA_EAUD_INVDEV;
    }
    
    param->clock_rate = adi.default_samples_per_sec;
    param->channel_count = 1;
    param->samples_per_frame = adi.default_samples_per_sec * 20 / 1000;
    param->bits_per_sample = 16;
    param->input_latency_ms = PJMEDIA_SND_DEFAULT_REC_LATENCY;
    param->output_latency_ms = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
    
    return PJ_SUCCESS;
}

pj_status_t OhaudioCreateStream(pjmedia_aud_dev_factory *f,
                                const pjmedia_aud_param *param,
                                pjmedia_aud_rec_cb rec_cb,
                                pjmedia_aud_play_cb play_cb,
                                void *user_data,pjmedia_aud_stream **p_aud_strm){
    NLOGI("AudioBackend OHA OhaudioCreateStream start");
    ohaudio_factory *ohFactory = reinterpret_cast<ohaudio_factory *>(f);
    pj_pool_t *pool = pj_pool_create(ohFactory->pf, "oh_audio_stream", 1024, 1024, nullptr);
    if(pool == nullptr){
        return PJ_EINVAL;
    }
    
    ohaudio_stream *strm;
    strm = PJ_POOL_ZALLOC_T(pool, ohaudio_stream);
    strm->pool = pool;
    pj_strdup2_with_null(pool, &strm->name, "oh_audio_stream");
    strm->dir = param->dir;
    pj_memcpy(&strm->param, param, sizeof(*param));
    
    strm->user_data = user_data;
    strm->recCallback = rec_cb;
    strm->playCallback = play_cb;
    
    if (strm->dir & PJMEDIA_DIR_PLAYBACK){//播放
        strm->renderer = ohFactory->renderer;
        
    }
    
    if(strm->dir & PJMEDIA_DIR_CAPTURE){//采集
        strm->capture = ohFactory->capture;
        
    }
    
    strm->base.op = &stream_operations;
    *p_aud_strm = &strm->base;
    NLOGI("AudioBackend OHA OhaudioCreateStream end");
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


