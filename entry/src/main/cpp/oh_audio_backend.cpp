//
// Created panyi
// 
//

#include "oh_audio_backend.h"

pj_status_t OhAudioBackEnd::factoryInit(pjmedia_aud_dev_factory *f){
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::factoryDestroy(pjmedia_aud_dev_factory *f){
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::factoryRefresh(pjmedia_aud_dev_factory *f) {
    return PJ_SUCCESS;
}

unsigned OhAudioBackEnd::factoryGetDevCount(pjmedia_aud_dev_factory *f) {
    return 1;
}

pj_status_t OhAudioBackEnd::factoryGetDevInfo(pjmedia_aud_dev_factory *f,
                                            unsigned index,
                                            pjmedia_aud_dev_info *info) {
    return PJ_SUCCESS;    
}


pj_status_t OhAudioBackEnd::factoryDefaultParam(pjmedia_aud_dev_factory *f,
                                                 unsigned index,
                                                 pjmedia_aud_param *param) {
    return PJ_SUCCESS;
}


pj_status_t OhAudioBackEnd::factoryCreateStream(pjmedia_aud_dev_factory *f,
                                 const pjmedia_aud_param *param,
                                 pjmedia_aud_rec_cb rec_cb,
                                 pjmedia_aud_play_cb play_cb,
                                 void *user_data,
                                 pjmedia_aud_stream **p_strm){
    return PJ_SUCCESS;
}

/* ---------- 流操作 ---------- */
pj_status_t OhAudioBackEnd::streamStart(pjmedia_aud_stream *strm) {
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::streamStop(pjmedia_aud_stream *strm) {
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::streamDestroy(pjmedia_aud_stream *strm) {
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::streamGetParam(pjmedia_aud_stream *strm,
                                        pjmedia_aud_param *param) {
    return PJ_SUCCESS;     
}

pj_status_t OhAudioBackEnd::streamGetCap(pjmedia_aud_stream *strm,
                                        pjmedia_aud_dev_cap cap,
                                        void *value){
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::streamSetCap(pjmedia_aud_stream *strm,
                                        pjmedia_aud_dev_cap cap,
                                        const void *value) {
    return PJ_SUCCESS;    
}
