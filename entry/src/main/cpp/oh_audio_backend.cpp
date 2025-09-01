//
// Created panyi
// 
//

#include "oh_audio_backend.h"
#include "log.h"

struct oh_factory{
    pjmedia_aud_dev_factory base;
    pj_pool_factory        *pf;
    pj_pool_t              *pool;
};

struct oh_stream{
    
};

pj_status_t oh_init(pjmedia_aud_dev_factory *f);

pj_status_t oh_destroy(pjmedia_aud_dev_factory *f);

pj_status_t oh_refresh(pjmedia_aud_dev_factory *f);

unsigned oh_get_dev_count(pjmedia_aud_dev_factory *f);

pj_status_t oh_get_dev_info(pjmedia_aud_dev_factory *f,
                                            unsigned index,
                                            pjmedia_aud_dev_info *info);

pj_status_t oh_default_param(pjmedia_aud_dev_factory *f,
                                             unsigned index,
                                             pjmedia_aud_param *param);

pj_status_t oh_create_stream(pjmedia_aud_dev_factory *f,
                                             const pjmedia_aud_param *param,
                                             pjmedia_aud_rec_cb rec_cb,
                                             pjmedia_aud_play_cb play_cb,
                                             void *user_data,
                                             pjmedia_aud_stream **p_strm);

static pjmedia_aud_dev_factory_op oh_factory_op = {
    &oh_init,
    &oh_destroy,
    &oh_get_dev_count,
    &oh_get_dev_info,
    &oh_default_param,
    &oh_create_stream,
    &oh_refresh
};

  
pj_status_t oh_strm_start(pjmedia_aud_stream *strm);
pj_status_t oh_strm_stop(pjmedia_aud_stream *strm);
pj_status_t oh_strm_destroy(pjmedia_aud_stream *strm);
pj_status_t oh_strm_get_param(pjmedia_aud_stream *strm,pjmedia_aud_param *param);
pj_status_t oh_strm_get_cap(pjmedia_aud_stream *strm,pjmedia_aud_dev_cap cap,void *value);
pj_status_t oh_strm_set_cap(pjmedia_aud_stream *strm,pjmedia_aud_dev_cap cap,const void *value);

static pjmedia_aud_stream_op oh_strm_op = {
    &oh_strm_get_param,
    &oh_strm_get_cap,
    &oh_strm_set_cap,
    &oh_strm_start,
    &oh_strm_stop,
    &oh_strm_destroy
};

pj_status_t oh_init(pjmedia_aud_dev_factory *f) {
    NLOGI("oh_init");
    return PJ_SUCCESS;
}

pj_status_t oh_destroy(pjmedia_aud_dev_factory *f) {
    NLOGI("oh_destroy");
    return PJ_SUCCESS;
}

pj_status_t oh_refresh(pjmedia_aud_dev_factory *f) {
    NLOGI("oh_refresh");
    return PJ_SUCCESS;
}

unsigned oh_get_dev_count(pjmedia_aud_dev_factory *f) {
    NLOGI("oh_get_dev_count");
    return 1;
}

pj_status_t oh_get_dev_info(pjmedia_aud_dev_factory *f,
                                            unsigned index,
                                            pjmedia_aud_dev_info *info){
    NLOGI("oh_get_dev_info");
    return PJ_SUCCESS;
}

pj_status_t oh_default_param(pjmedia_aud_dev_factory *f,
                                             unsigned index,
                                             pjmedia_aud_param *param){
    NLOGI("oh_default_param");
    return PJ_SUCCESS;
}

pj_status_t oh_create_stream(pjmedia_aud_dev_factory *f,
                                             const pjmedia_aud_param *param,
                                             pjmedia_aud_rec_cb rec_cb,
                                             pjmedia_aud_play_cb play_cb,
                                             void *user_data,
                                             pjmedia_aud_stream **p_strm){
    NLOGI("oh_create_stream");
    return PJ_SUCCESS;
}

/* ---------- 流操作 ---------- */
pj_status_t oh_strm_start(pjmedia_aud_stream *strm) {
    NLOGI("oh_strm_start");
    return PJ_SUCCESS;
}

pj_status_t oh_strm_stop(pjmedia_aud_stream *strm) {
    NLOGI("oh_strm_stop");
    return PJ_SUCCESS;
}

pj_status_t oh_strm_destroy(pjmedia_aud_stream *strm) {
    NLOGI("oh_strm_destroy");
    return PJ_SUCCESS;
}

pj_status_t oh_strm_get_param(pjmedia_aud_stream *strm,pjmedia_aud_param *param){
    NLOGI("oh_strm_get_param");
    return PJ_SUCCESS;
}

pj_status_t oh_strm_get_cap(pjmedia_aud_stream *strm,pjmedia_aud_dev_cap cap,void *value){
    NLOGI("oh_strm_get_cap");
    return PJ_SUCCESS;
}

pj_status_t oh_strm_set_cap(pjmedia_aud_stream *strm,pjmedia_aud_dev_cap cap,const void *value) {
    NLOGI("oh_strm_set_cap");
    return PJ_SUCCESS;
}

pjmedia_aud_dev_factory* OhAudioBackEnd::buildFactory(pj_pool_factory *pf){
    NLOGI("buildFactory");
    struct oh_factory *f;
    pj_pool_t *pool;
    
    pool = pj_pool_create(pf, "oh_factory_pool", 1024, 1024, NULL);
    f = PJ_POOL_ZALLOC_T(pool, struct oh_factory);
    f->pf = pf;
    f->pool = pool;
    f->base.op = &oh_factory_op;
    return &f->base;
}

pj_status_t OhAudioBackEnd::factoryInit(pjmedia_aud_dev_factory *f){
    NLOGI("factoryInit");
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::factoryDestroy(pjmedia_aud_dev_factory *f){
    NLOGI("factoryDestroy");
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::factoryRefresh(pjmedia_aud_dev_factory *f) {
    NLOGI("factoryRefresh");
    
    return PJ_SUCCESS;
}

unsigned OhAudioBackEnd::factoryGetDevCount(pjmedia_aud_dev_factory *f) {
    NLOGI("factoryGetDevCount");
    return 1;
}

pj_status_t OhAudioBackEnd::factoryGetDevInfo(pjmedia_aud_dev_factory *f,
                                            unsigned index,
                                            pjmedia_aud_dev_info *info) {
    NLOGI("factoryGetDevInfo");
    return PJ_SUCCESS;    
}


pj_status_t OhAudioBackEnd::factoryDefaultParam(pjmedia_aud_dev_factory *f,
                                                 unsigned index,
                                                 pjmedia_aud_param *param) {
    NLOGI("factoryDefaultParam");
    return PJ_SUCCESS;
}


pj_status_t OhAudioBackEnd::factoryCreateStream(pjmedia_aud_dev_factory *f,
                                 const pjmedia_aud_param *param,
                                 pjmedia_aud_rec_cb rec_cb,
                                 pjmedia_aud_play_cb play_cb,
                                 void *user_data,
                                 pjmedia_aud_stream **p_strm){
    NLOGI("factoryCreateStream");
    return PJ_SUCCESS;
}


pj_status_t OhAudioBackEnd::streamStart(pjmedia_aud_stream *strm) {
    NLOGI("streamStart");
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::streamStop(pjmedia_aud_stream *strm) {
    NLOGI("streamStop");
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::streamDestroy(pjmedia_aud_stream *strm) {
    NLOGI("streamDestroy");
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::streamGetParam(pjmedia_aud_stream *strm,
                                        pjmedia_aud_param *param) {
    NLOGI("streamGetParam");
    return PJ_SUCCESS;     
}

pj_status_t OhAudioBackEnd::streamGetCap(pjmedia_aud_stream *strm,
                                        pjmedia_aud_dev_cap cap,
                                        void *value){
    NLOGI("streamGetCap");
    return PJ_SUCCESS;
}

pj_status_t OhAudioBackEnd::streamSetCap(pjmedia_aud_stream *strm,
                                        pjmedia_aud_dev_cap cap,
                                        const void *value) {
    NLOGI("streamSetCap");
    return PJ_SUCCESS;    
}
