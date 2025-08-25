export const add: (a: number, b: number) => number;

export const getNativeString:() => string;

export const getPjsipVersionStr:() => string;

export const findCodecFormats:()=>string;

export const registerCallback:(cb:()=>void) => void;

export const registerCallback2:(value:number, cb:(counter:number)=>void) => void;

export const SipAppStart:()=>void;

export const SipAppEnd:()=>void;

export const SipLogin:(account:string,password:string)=>void;
