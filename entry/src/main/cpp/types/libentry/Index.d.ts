export const add: (a: number, b: number) => number;

export const getNativeString:() => string;

export const getPjsipVersionStr:() => string;

export const findCodecFormats:()=>string;

export const registerCallback:(cb:()=>void) => void;

export const registerCallback2:(value:number, cb:(counter:number)=>void) => void;

export const SipAppStart:()=>void;

export const SipAppEnd:()=>void;

export const SipLogin:(account:string,password:string)=>void;

export const RegisterSipObserver:(observer:object)=>void;

export const UnregisterSipObserver:(observer:object)=>void;

export const SipCallAccept:(callId:string)=>void;

export const SipCallHangup:(callId:string, isBusy:boolean)=>void;

export const SipMakeCall:(call_number:string)=>string;



