//
// Created on 2025/9/1.
//
#ifndef _AUDIO_BACKEND_OHA_H_
#define _AUDIO_BACKEND_OHA_H_

#include "audio_backend_default.h"

class AudioBackendOha: public AudioBackendDefault {
public:
    virtual pjmedia_aud_dev_factory* buildFactory(pj_pool_factory *pf) override;
};

#endif //_AUDIO_BACKEND_OHA_H_
