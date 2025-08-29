//
// Created on 2025/8/25.
// panyi

#include "sip_app.h"
#include "my_call.h"
#include "log.h"

void MyCall::onCallState(pj::OnCallStateParam &prm) {
    Call::onCallState(prm);
    pj::CallInfo info = getInfo();
    NLOGI("MyCall on Call state change state: %{public}d", info.state);
    if(info.state == PJSIP_INV_STATE_DISCONNECTED){
        std::string callId = getCallId();
        appContext->removeCall(callId);
        
        appContext->fireObserverCallback(OBSERVER_METHOD_DISCONNECT, -1, getCallId());
    }
}

bool MyCall::isDisconnect(){
    return getInfo().state == PJSIP_INV_STATE_DISCONNECTED;
}

void MyCall::onCallMediaState(pj::OnCallMediaStateParam &prm){
    pj::CallInfo ci = getInfo();
    NLOGI("MyCall onCallMediaState state: %{public}d", ci.state);
    
    try{
        // NLOGI("MyCall try getAudioMedia");
        // pj::AudioMedia audMed = getAudioMedia(-1);

        // NLOGI("MyCall try startTransmit");
        // appContext->getEndpoint()->audDevManager().getCaptureDevMedia().startTransmit(audMed);

        // NLOGI("MyCall try getPlaybackDevMedia");
        // audMed.startTransmit(appContext->getEndpoint()->audDevManager().getPlaybackDevMedia());
        
        // NLOGI("MyCall onCallMediaState Start");

        for (unsigned i = 0; i < ci.media.size(); i++){
            if (ci.media[i].type == PJMEDIA_TYPE_AUDIO &&
                (ci.media[i].status == PJSUA_CALL_MEDIA_ACTIVE ||
                 ci.media[i].status == PJSUA_CALL_MEDIA_REMOTE_HOLD)){
                NLOGI("MyCall is Right State index:%{public}d",i);
                
                pj::AudioMedia aud_med = getAudioMedia(i);
                
                pj::AudDevManager& adm = appContext->getEndpoint()->audDevManager();
                int capDev = adm.getCaptureDev();
                int playDev = adm.getPlaybackDev();
                NLOGI("CaptureDev=%{public}d, PlaybackDev=%{public}d", capDev, playDev);
                
                NLOGI("MyCall try getPlaybackDevMedia");
                aud_med.startTransmit(adm.getPlaybackDevMedia());
                NLOGI("MyCall try startTransmit getPlaybackDevMedia end");
                
                NLOGI("MyCall try getCaptureDevMedia");
                adm.getCaptureDevMedia().startTransmit(aud_med);
                NLOGI("MyCall try startTransmit end");
                                
                NLOGI("MyCall fireObserverCallback Started");
                appContext->fireObserverCallback(OBSERVER_METHOD_START_AUDIO, -1, getCallId());
                break;
            }
        }//end for i


        
    } catch (std::exception &e) {
        NLOGE("MyCall onCallMediaState ERROR:${public}s", e.what());
    }
}