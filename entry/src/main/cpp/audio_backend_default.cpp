//
// Created panyi
// 
//

#include "audio_backend_default.h"
#include "log.h"

#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_OpenHarmony.h"
#include "pjsua2.hpp"

#define W_SLBufferQueueItf SLOHBufferQueueItf
#define W_SL_IID_BUFFERQUEUE SL_IID_OH_BUFFERQUEUE

#define NUM_BUFFERS 2

typedef long pj_thread_desc[64];

struct opensl_aud_factory{
    pjmedia_aud_dev_factory  base;
    pj_pool_factory         *pf;
    pj_pool_t               *pool;
    
    SLObjectItf              engineObject;
    SLEngineItf              engineEngine;
    SLObjectItf              outputMixObject;
};

struct opensl_aud_stream
{
    pjmedia_aud_stream  base;
    pj_pool_t          *pool;
    pj_str_t            name;
    pjmedia_dir         dir;
    pjmedia_aud_param   param;
    
    void               *user_data;
    pj_bool_t           quit_flag;
    pjmedia_aud_rec_cb  rec_cb;
    pjmedia_aud_play_cb play_cb;

    pj_timestamp        play_timestamp;
    pj_timestamp        rec_timestamp;
    
    pj_bool_t           rec_thread_initialized;
    pj_thread_desc      rec_thread_desc;
    pj_thread_t        *rec_thread;
    
    pj_bool_t           play_thread_initialized;
    pj_thread_desc      play_thread_desc;
    pj_thread_t        *play_thread;
    
    /* Player */
    SLObjectItf         playerObj;
    SLPlayItf           playerPlay;
    SLVolumeItf         playerVol;
    unsigned            playerBufferSize;
    char               *playerBuffer[NUM_BUFFERS];
    int                 playerBufIdx;
    
    /* Recorder */
    SLObjectItf         recordObj;
    SLRecordItf         recordRecord;
    unsigned            recordBufferSize;
    char               *recordBuffer[NUM_BUFFERS];
    int                 recordBufIdx;

    W_SLBufferQueueItf  playerBufQ;
    W_SLBufferQueueItf  recordBufQ;
};

pj_status_t opensl_to_pj_error(SLresult code);

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

void bqPlayerCallback(W_SLBufferQueueItf bq, void *context ,SLuint32 size){
    struct opensl_aud_stream *stream = (struct opensl_aud_stream*) context;
    SLresult result;
    int status;

    if (stream->play_thread_initialized == 0 || !pj_thread_is_registered())
    {
        pj_bzero(stream->play_thread_desc, sizeof(pj_thread_desc));
        status = pj_thread_register("opensl_play", stream->play_thread_desc,
                                    &stream->play_thread);
        stream->play_thread_initialized = 1;
        NLOGI("Player thread started");
    }
    
    if (!stream->quit_flag) {
        pjmedia_frame frame;
        char * buf;
        
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        frame.buf = buf = stream->playerBuffer[stream->playerBufIdx++];
        frame.size = stream->playerBufferSize;
        frame.timestamp.u64 = stream->play_timestamp.u64;
        frame.bit_info = 0;
        
        status = (*stream->play_cb)(stream->user_data, &frame);
        if (status != PJ_SUCCESS || frame.type != PJMEDIA_FRAME_TYPE_AUDIO)
            pj_bzero(buf, stream->playerBufferSize);
        
        stream->play_timestamp.u64 += stream->param.samples_per_frame /
                                      stream->param.channel_count;
        
        result = (*bq)->Enqueue(bq, buf, stream->playerBufferSize);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Unable to enqueue next player buffer !!! %{public}d", result);
        }
        
        stream->playerBufIdx %= NUM_BUFFERS;
    }
}


/* This callback handler is called every time a buffer finishes recording */
void bqRecorderCallback(W_SLBufferQueueItf bq, void *context,SLuint32 size){
    NLOGI("bqRecorderCallback called");
    
    struct opensl_aud_stream *stream = (struct opensl_aud_stream*) context;
    SLresult result;
    int status;

    pj_assert(context != NULL);
    pj_assert(bq == stream->recordBufQ);
    
    if (stream->rec_thread_initialized == 0 || !pj_thread_is_registered()){
        pj_bzero(stream->rec_thread_desc, sizeof(pj_thread_desc));
        status = pj_thread_register("opensl_rec", stream->rec_thread_desc,
                                    &stream->rec_thread);
        PJ_UNUSED_ARG(status);  /* Unused for now.. */
        stream->rec_thread_initialized = 1;
        NLOGI("Recorder thread started");
    }
    
    if (!stream->quit_flag) {
        pjmedia_frame frame;
        unsigned char *buf;
        
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        (*bq)->GetBuffer(bq, &buf, &size);
//        buf = stream->recordBuffer[stream->recordBufIdx++];
        frame.buf = buf;
        
        NLOGI("capture record buffsize %{public}d  size = %{public}d"
                ,stream->recordBufferSize, size);
        NLOGI("capture buffer data:%{public}d %{public}d %{public}d %{public}d %{public}d %{public}d %{public}d %{public}d"
                ,buf[10],buf[11],buf[12],buf[13]
                ,buf[14],buf[15],buf[16],buf[17]);
        
        frame.size = size;
        frame.timestamp.u64 = stream->rec_timestamp.u64;
        frame.bit_info = 0;
        
        status = (*stream->rec_cb)(stream->user_data, &frame);
        
        stream->rec_timestamp.u64 += stream->param.samples_per_frame /
                                     stream->param.channel_count;
        
        /* And now enqueue next buffer */
        result = (*bq)->Enqueue(bq, buf, size);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Unable to enqueue next record buffer !!! %{public}d",result);
        }
        
        stream->recordBufIdx %= NUM_BUFFERS;
        NLOGI("bqRecorderCallback called ended.");
    }
}

pj_status_t oh_init(pjmedia_aud_dev_factory *f) {
    NLOGI("oh_init");
    
    struct opensl_aud_factory *pa = (struct opensl_aud_factory*)f;
    SLresult result;    
    
    /* Create engine */
    result = slCreateEngine(&pa->engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        NLOGE("Cannot create engine %{public}d", result);
        return opensl_to_pj_error(result);
    }
    
    /* Realize the engine */
    result = (*pa->engineObject)->Realize(pa->engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        NLOGE("Cannot realize engine");
        oh_destroy(f);
        return opensl_to_pj_error(result);
    }
    
    result = (*pa->engineObject)->GetInterface(pa->engineObject,
                                               SL_IID_ENGINE,
                                               &pa->engineEngine);
    if (result != SL_RESULT_SUCCESS) {
        oh_destroy(f);
        return opensl_to_pj_error(result);
    }
    
    /* Create output mix */
    result = (*pa->engineEngine)->CreateOutputMix(pa->engineEngine,
                                                  &pa->outputMixObject,
                                                  0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        NLOGE("Cannot create output mix");
        oh_destroy(f);
        return opensl_to_pj_error(result);
    }
    
    /* Realize the output mix */
    result = (*pa->outputMixObject)->Realize(pa->outputMixObject,
                                             SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        NLOGE("Cannot realize output mix");
        oh_destroy(f);
        return opensl_to_pj_error(result);
    }

    NLOGI("Create Opensl init Success!!");
    return PJ_SUCCESS;
}

pj_status_t oh_destroy(pjmedia_aud_dev_factory *f) {
    NLOGI("oh_destroy");
    
    struct opensl_aud_factory *pa = (struct opensl_aud_factory*)f;
    pj_pool_t *pool;
    
    NLOGI("OpenSL sound library shutting down... ...");
    
    /* Destroy Output Mix object */
    if (pa->outputMixObject) {
        (*pa->outputMixObject)->Destroy(pa->outputMixObject);
        pa->outputMixObject = nullptr;
    }
    
    /* Destroy engine object, and invalidate all associated interfaces */
    if (pa->engineObject) {
        (*pa->engineObject)->Destroy(pa->engineObject);
        pa->engineObject = nullptr;
        pa->engineEngine = nullptr;
    }
    
    pool = pa->pool;
    pa->pool = nullptr;
    pj_pool_release(pool);
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
    pj_bzero(info, sizeof(*info));
    
    pj_ansi_strcpy(info->name, "OH_OpenSL ES Audio");
    info->default_samples_per_sec = 8000;
    info->caps = PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING;
    info->input_count = 1;
    info->output_count = 1;
    
    return PJ_SUCCESS;
}

pj_status_t oh_default_param(pjmedia_aud_dev_factory *f,
                                             unsigned index,
                                             pjmedia_aud_param *param){
    NLOGI("oh_default_param");
    pjmedia_aud_dev_info adi;
    pj_status_t status;
    
    status = oh_get_dev_info(f, index, &adi);
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

pj_status_t oh_create_stream(pjmedia_aud_dev_factory *f,
                                             const pjmedia_aud_param *param,
                                             pjmedia_aud_rec_cb rec_cb,
                                             pjmedia_aud_play_cb play_cb,
                                             void *user_data,
                                             pjmedia_aud_stream **p_aud_strm){
    NLOGI("oh_create_stream");
    SLDataLocator_BufferQueue loc_bq ={ SL_DATALOCATOR_BUFFERQUEUE, NUM_BUFFERS };
    struct opensl_aud_factory *pa = (struct opensl_aud_factory*)f;
    pj_pool_t *pool;
    struct opensl_aud_stream *stream;
    int i, bufferSize;
    SLresult result;
    SLDataFormat_PCM format_pcm;
    
    NLOGI("Creating OpenSL stream");
    
    pool = pj_pool_create(pa->pf, "openslstrm", 1024, 1024, NULL);
    if (!pool){
        return PJ_ENOMEM;    
    }
    
    stream = PJ_POOL_ZALLOC_T(pool, struct opensl_aud_stream);
    stream->pool = pool;
    
    pj_strdup2_with_null(pool, &stream->name, "OpenSL");
    stream->dir = param->dir;
    
    pj_memcpy(&stream->param, param, sizeof(*param));
    stream->user_data = user_data;
    stream->rec_cb = rec_cb;
    stream->play_cb = play_cb;
    bufferSize = param->samples_per_frame * param->bits_per_sample / 8;
    
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = param->channel_count;
    
    format_pcm.samplesPerSec  = (SLuint32) param->clock_rate * 1000;
    format_pcm.bitsPerSample = (SLuint16) param->bits_per_sample;
    format_pcm.containerSize = (SLuint16) param->bits_per_sample;
    format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;
    
    if (stream->dir & PJMEDIA_DIR_PLAYBACK) {//播放流
        SLDataSource audioSrc = {&loc_bq, &format_pcm};
        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX,pa->outputMixObject};
        SLDataSink audioSnk = {&loc_outmix, nullptr};
        
        int numIface = 2;
        const SLInterfaceID ids[2] = {W_SL_IID_BUFFERQUEUE,SL_IID_VOLUME};
        const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
        
        //创建播放器
        result = (*pa->engineEngine)->CreateAudioPlayer(pa->engineEngine,
                                                        &stream->playerObj,
                                                        &audioSrc, &audioSnk,
                                                        numIface, ids, req);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot create audio player: %{public}d", result);
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        
        result = (*stream->playerObj)->Realize(stream->playerObj,SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot realize player : %{public}d", result);
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        NLOGI("Create audio player success");
        
        result = (*stream->playerObj)->GetInterface(stream->playerObj,
                                                    SL_IID_PLAY,
                                                    &stream->playerPlay);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot get play interface");
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        
        result = (*stream->playerObj)->GetInterface(stream->playerObj,
                                                    W_SL_IID_BUFFERQUEUE,
                                                    &stream->playerBufQ);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot get buffer queue interface");
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        
         NLOGI("get buffer queue success");
        
        result = (*stream->playerObj)->GetInterface(stream->playerObj,
                                            SL_IID_VOLUME,
                                            &stream->playerVol);
        NLOGI("RegisterCallback");
        result = (*stream->playerBufQ)->RegisterCallback(stream->playerBufQ,
                                                         bqPlayerCallback,
                                                          (void *)stream);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot register player callback");
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        
        NLOGI("RegisterCallback SUCCESS");
        stream->playerBufferSize = bufferSize;
        for (i = 0; i < NUM_BUFFERS; i++) {
            stream->playerBuffer[i] = (char *)pj_pool_alloc(stream->pool,stream->playerBufferSize);
        }//end for i
    }
    
    if (stream->dir & PJMEDIA_DIR_CAPTURE){ //采集音频
        NLOGI("oh_strm start to set capture");
        SLDataLocator_IODevice loc_dev = {
                                    SL_DATALOCATOR_IODEVICE,
                                    SL_IODEVICE_AUDIOINPUT,
                                    SL_DEFAULTDEVICEID_AUDIOINPUT,
                                    nullptr};
        SLDataSource audioSrc = {&loc_dev, nullptr};
        SLDataSink audioSnk = {&loc_bq, &format_pcm};
        
        int numIface = 1;
        const SLInterfaceID ids[1] = {W_SL_IID_BUFFERQUEUE};
        const SLboolean req[1] = {SL_BOOLEAN_TRUE};
        
        result = (*pa->engineEngine)->CreateAudioRecorder(pa->engineEngine,
                                                          &stream->recordObj,
                                                          &audioSrc, &audioSnk,
                                                          numIface, ids, req);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot create recorder: %{public}d", result);
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        
        result = (*stream->recordObj)->Realize(stream->recordObj,SL_BOOLEAN_FALSE);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot realize recorder : %{public}d",result);
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        
        result = (*stream->recordObj)->GetInterface(stream->recordObj,
                                            SL_IID_RECORD,
                                            &stream->recordRecord);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot get record interface");
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        
        result = (*stream->recordObj)->GetInterface(
                     stream->recordObj, W_SL_IID_BUFFERQUEUE,
                     &stream->recordBufQ);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot get recorder buffer queue iface");
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        NLOGI("oh_strm capture RegisterCallback");
        result = (*stream->recordBufQ)->RegisterCallback(stream->recordBufQ,
                                                         bqRecorderCallback, 
                                                         (void *) stream);
        
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot register recorder callback");
            oh_strm_destroy(&stream->base);
            return opensl_to_pj_error(result);
        }
        
        NLOGI("oh_strm capture RegisterCallback success");
        stream->recordBufferSize = bufferSize;
        for (i = 0; i < NUM_BUFFERS; i++) {
            stream->recordBuffer[i] = (char *)pj_pool_alloc(stream->pool,stream->recordBufferSize);
        }//end for i;
    
        NLOGI("oh_strm capture recordBufferSize %{public}d", stream->recordBufferSize);
    }
    
    if (param->flags & PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING) {
        oh_strm_set_cap(&stream->base, 
                        PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING,
                     &param->output_vol);
    }
    
    /* Done */
    stream->base.op = &oh_strm_op;
    *p_aud_strm = &stream->base;
    return PJ_SUCCESS;
}

/* ---------- 流操作 ---------- */
pj_status_t oh_strm_get_param(pjmedia_aud_stream *s,pjmedia_aud_param *pi){
    NLOGI("oh_strm_get_param");
    struct opensl_aud_stream *strm = (struct opensl_aud_stream*)s;
    
    pj_memcpy(pi, &strm->param, sizeof(*pi));
    
    if (oh_strm_get_cap(s, PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING,
                     &pi->output_vol) == PJ_SUCCESS){
        pi->flags |= PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING;
    }  
    return PJ_SUCCESS;
}

pj_status_t oh_strm_get_cap(pjmedia_aud_stream *s,pjmedia_aud_dev_cap cap,void *pval){
    NLOGI("oh_strm_get_cap");
    struct opensl_aud_stream *strm = (struct opensl_aud_stream*)s;
    pj_status_t status = PJMEDIA_EAUD_INVCAP;
     if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING &&(strm->param.dir & PJMEDIA_DIR_PLAYBACK)){
        if (strm->playerVol) {
            SLresult res;
            SLmillibel vol, mvol;
            res = (*strm->playerVol)->GetMaxVolumeLevel(strm->playerVol,&mvol);
            if (res == SL_RESULT_SUCCESS) {
                res = (*strm->playerVol)->GetVolumeLevel(strm->playerVol,&vol);
                if (res == SL_RESULT_SUCCESS) {
                    *(int *)pval = ((int)vol - SL_MILLIBEL_MIN) * 100 /
                                   ((int)mvol - SL_MILLIBEL_MIN);
                    return PJ_SUCCESS;
                }
            }
        }
    }
    return status;
}

pj_status_t oh_strm_set_cap(pjmedia_aud_stream *s,pjmedia_aud_dev_cap cap,const void *value) {
    NLOGI("oh_strm_set_cap");
    struct opensl_aud_stream *strm = (struct opensl_aud_stream*)s;
    
    if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING && (strm->param.dir & PJMEDIA_DIR_PLAYBACK)){
        if (strm->playerVol){
            SLresult res;
            SLmillibel vol, mvol;
            res = (*strm->playerVol)->GetMaxVolumeLevel(strm->playerVol,&mvol);
            if (res == SL_RESULT_SUCCESS) {
                vol = (SLmillibel)(*(int *)value *
                      ((int)mvol - SL_MILLIBEL_MIN) / 100 + SL_MILLIBEL_MIN);
                res = (*strm->playerVol)->SetVolumeLevel(strm->playerVol,
                                                         vol);
                if (res == SL_RESULT_SUCCESS)
                    return PJ_SUCCESS;
            }
        }
    }
    return PJMEDIA_EAUD_INVCAP;
}

pj_status_t oh_strm_start(pjmedia_aud_stream *s) {
    NLOGI("oh_strm_start");
    struct opensl_aud_stream *stream = (struct opensl_aud_stream*)s;
    
    int i;
    SLresult result = SL_RESULT_SUCCESS;
    NLOGI("Starting %{public}s stream..", stream->name.ptr);
    stream->quit_flag = 0;
    
    if (stream->recordBufQ && stream->recordRecord){
        for (i = 0; i < NUM_BUFFERS; i++) {
            result = (*stream->recordBufQ)->Enqueue(stream->recordBufQ,
                                                stream->recordBuffer[i],
                                                stream->recordBufferSize);
            pj_assert(result == SL_RESULT_SUCCESS);
        }//end for i
        
        result = (*stream->recordRecord)->SetRecordState(stream->recordRecord, SL_RECORDSTATE_RECORDING);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot start recorder");
            oh_strm_stop(&stream->base);
            return opensl_to_pj_error(result);
        }
    }
    
    if (stream->playerPlay && stream->playerBufQ){
        result = (*stream->playerPlay)->SetPlayState(stream->playerPlay,SL_PLAYSTATE_PLAYING);
        if (result != SL_RESULT_SUCCESS) {
            NLOGE("Cannot start player");
            oh_strm_stop(&stream->base);
            return opensl_to_pj_error(result);
        }
        
        for (i = 0; i < NUM_BUFFERS; i++) {
            pj_bzero(stream->playerBuffer[i], stream->playerBufferSize/100);
            result = (*stream->playerBufQ)->Enqueue(stream->playerBufQ,
                                                stream->playerBuffer[i],
                                                stream->playerBufferSize/100);
            pj_assert(result == SL_RESULT_SUCCESS);
        }
    }
    
    NLOGI("%{public}s stream started", stream->name.ptr);
    return PJ_SUCCESS;
}

pj_status_t oh_strm_stop(pjmedia_aud_stream *s) {
    NLOGI("oh_strm_stop");
    struct opensl_aud_stream *stream = (struct opensl_aud_stream*)s;
    
     if (stream->quit_flag)
        return PJ_SUCCESS;
    
    NLOGI("Stopping stream");
    
    stream->quit_flag = 1;
    
    if (stream->recordBufQ && stream->recordRecord) {
        /* Stop recording and clear buffer queue */
        (*stream->recordRecord)->SetRecordState(stream->recordRecord,SL_RECORDSTATE_STOPPED);
        (*stream->recordBufQ)->Clear(stream->recordBufQ);
    }
    
    if (stream->playerBufQ && stream->playerPlay){
        /* Wait until the PCM data is done playing, the buffer queue callback
         * will continue to queue buffers until the entire PCM data has been
         * played. This is indicated by waiting for the count member of the
         * SLBufferQueueState to go to zero.
         */
        /*      
        SLresult result;
        W_SLBufferQueueState state;

        result = (*stream->playerBufQ)->GetState(stream->playerBufQ, &state);
        while (state.count) {
            (*stream->playerBufQ)->GetState(stream->playerBufQ, &state);
        } */
        /* Stop player */
        (*stream->playerPlay)->SetPlayState(stream->playerPlay,SL_PLAYSTATE_STOPPED);
    }
    
    NLOGI("OpenSL stream stopped");
    return PJ_SUCCESS;
}

pj_status_t oh_strm_destroy(pjmedia_aud_stream *s) {
    NLOGI("oh_strm_destroy");
    struct opensl_aud_stream *stream = (struct opensl_aud_stream*)s;
    oh_strm_stop(s);
    
    if (stream->playerObj) {
        /* Destroy the player */
        (*stream->playerObj)->Destroy(stream->playerObj);
        /* Invalidate all associated interfaces */
        stream->playerObj = nullptr;
        stream->playerPlay = nullptr;
        stream->playerBufQ = nullptr;
        stream->playerVol = nullptr;
    }
    
    if (stream->recordObj) {
        /* Destroy the recorder */
        (*stream->recordObj)->Destroy(stream->recordObj);
        /* Invalidate all associated interfaces */
        stream->recordObj = nullptr;
        stream->recordRecord = nullptr;
        stream->recordBufQ = nullptr;
    }
    
    pj_pool_release(stream->pool);
    NLOGI("OpenSL stream destroyed");
    
    return PJ_SUCCESS;
}

pjmedia_aud_dev_factory* AudioBackendDefault::buildFactory(pj_pool_factory *pf){
    NLOGI("buildFactory");
    struct opensl_aud_factory *f;
    pj_pool_t *pool;
    
    pool = pj_pool_create(pf, "oh_opensles", 256, 256, NULL);
    f = PJ_POOL_ZALLOC_T(pool, struct opensl_aud_factory);
    f->pf = pf;
    f->pool = pool;
    f->base.op = &oh_factory_op;
    return &f->base;
}

pj_status_t AudioBackendDefault::factoryInit(pjmedia_aud_dev_factory *f){
    NLOGI("factoryInit");
    return PJ_SUCCESS;
}

pj_status_t AudioBackendDefault::factoryDestroy(pjmedia_aud_dev_factory *f){
    NLOGI("factoryDestroy");
    return PJ_SUCCESS;
}

pj_status_t AudioBackendDefault::factoryRefresh(pjmedia_aud_dev_factory *f) {
    NLOGI("factoryRefresh");
    
    return PJ_SUCCESS;
}

unsigned AudioBackendDefault::factoryGetDevCount(pjmedia_aud_dev_factory *f) {
    NLOGI("factoryGetDevCount");
    return 1;
}

pj_status_t AudioBackendDefault::factoryGetDevInfo(pjmedia_aud_dev_factory *f,
                                            unsigned index,
                                            pjmedia_aud_dev_info *info) {
    NLOGI("factoryGetDevInfo");
    return PJ_SUCCESS;    
}


pj_status_t AudioBackendDefault::factoryDefaultParam(pjmedia_aud_dev_factory *f,
                                                 unsigned index,
                                                 pjmedia_aud_param *param) {
    NLOGI("factoryDefaultParam");
    return PJ_SUCCESS;
}


pj_status_t AudioBackendDefault::factoryCreateStream(pjmedia_aud_dev_factory *f,
                                 const pjmedia_aud_param *param,
                                 pjmedia_aud_rec_cb rec_cb,
                                 pjmedia_aud_play_cb play_cb,
                                 void *user_data,
                                 pjmedia_aud_stream **p_strm){
    NLOGI("factoryCreateStream");
    return PJ_SUCCESS;
}


pj_status_t AudioBackendDefault::streamStart(pjmedia_aud_stream *strm) {
    NLOGI("streamStart");
    return PJ_SUCCESS;
}

pj_status_t AudioBackendDefault::streamStop(pjmedia_aud_stream *strm) {
    NLOGI("streamStop");
    return PJ_SUCCESS;
}

pj_status_t AudioBackendDefault::streamDestroy(pjmedia_aud_stream *strm) {
    NLOGI("streamDestroy");
    return PJ_SUCCESS;
}

pj_status_t AudioBackendDefault::streamGetParam(pjmedia_aud_stream *strm,
                                        pjmedia_aud_param *param) {
    NLOGI("streamGetParam");
    return PJ_SUCCESS;     
}

pj_status_t AudioBackendDefault::streamGetCap(pjmedia_aud_stream *strm,
                                        pjmedia_aud_dev_cap cap,
                                        void *value){
    NLOGI("streamGetCap");
    return PJ_SUCCESS;
}

pj_status_t AudioBackendDefault::streamSetCap(pjmedia_aud_stream *strm,
                                        pjmedia_aud_dev_cap cap,
                                        const void *value) {
    NLOGI("streamSetCap");
    return PJ_SUCCESS;    
}


pj_status_t opensl_to_pj_error(SLresult code){
    switch(code) {
        case SL_RESULT_SUCCESS:
            return PJ_SUCCESS;
        case SL_RESULT_PRECONDITIONS_VIOLATED:
        case SL_RESULT_PARAMETER_INVALID:
        case SL_RESULT_CONTENT_CORRUPTED:
        case SL_RESULT_FEATURE_UNSUPPORTED:
            return PJMEDIA_EAUD_INVOP;
        case SL_RESULT_MEMORY_FAILURE:
        case SL_RESULT_BUFFER_INSUFFICIENT:
            return PJ_ENOMEM;
        case SL_RESULT_RESOURCE_ERROR:
        case SL_RESULT_RESOURCE_LOST:
        case SL_RESULT_CONTROL_LOST:
            return PJMEDIA_EAUD_NOTREADY;
        case SL_RESULT_CONTENT_UNSUPPORTED:
            return PJ_ENOTSUP;
        default:
            return PJMEDIA_EAUD_ERR;
    }
}

