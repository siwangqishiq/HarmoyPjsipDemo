//
// Created on 2025/9/1.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef HARMOYPJSIPDEMO_OH_AUDIO_BACKEND_H
#define HARMOYPJSIPDEMO_OH_AUDIO_BACKEND_H

#include "pjmedia_custom_backend.h"

class OhAudioBackEnd: public CustomAudioBackend {
public:
    virtual pj_status_t factoryInit(pjmedia_aud_dev_factory *f) override;
    virtual pj_status_t factoryDestroy(pjmedia_aud_dev_factory *f) override;
    virtual pj_status_t factoryRefresh(pjmedia_aud_dev_factory *f) override;
    virtual unsigned factoryGetDevCount(pjmedia_aud_dev_factory *f) override;

    virtual pj_status_t factoryGetDevInfo(pjmedia_aud_dev_factory *f,
                                            unsigned index,
                                            pjmedia_aud_dev_info *info) override;

    virtual pj_status_t factoryDefaultParam(pjmedia_aud_dev_factory *f,
                                             unsigned index,
                                             pjmedia_aud_param *param) override;

    virtual pj_status_t factoryCreateStream(pjmedia_aud_dev_factory *f,
                                             const pjmedia_aud_param *param,
                                             pjmedia_aud_rec_cb rec_cb,
                                             pjmedia_aud_play_cb play_cb,
                                             void *user_data,
                                             pjmedia_aud_stream **p_strm) override;

    /* ---------- 流操作 ---------- */
    virtual pj_status_t streamStart(pjmedia_aud_stream *strm) override;

    virtual pj_status_t streamStop(pjmedia_aud_stream *strm) override;

    virtual pj_status_t streamDestroy(pjmedia_aud_stream *strm) override;

    virtual pj_status_t streamGetParam(pjmedia_aud_stream *strm,
                                        pjmedia_aud_param *param) override;

    virtual pj_status_t streamGetCap(pjmedia_aud_stream *strm,
                                        pjmedia_aud_dev_cap cap,
                                        void *value) override;

    virtual pj_status_t streamSetCap(pjmedia_aud_stream *strm,
                                        pjmedia_aud_dev_cap cap,
                                        const void *value) override;
};

#endif //HARMOYPJSIPDEMO_OH_AUDIO_BACKEND_H
