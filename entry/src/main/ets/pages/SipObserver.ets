export interface SipObserver{
  onRegStateChanged:(code:number, msg?:string)=>void

  onIncomingCall:(callId:string, info?:string)=>void

  onStartAudio:(callId:string)=>void

  onDisconnect:(callId:string)=>void
}