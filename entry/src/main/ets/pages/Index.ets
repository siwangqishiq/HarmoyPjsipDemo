import { hilog } from '@kit.PerformanceAnalysisKit';
import NativeBridge from 'libyxpjsip.so';
import { SipObserver } from './SipObserver';

const DOMAIN = 0x0000;

@Entry
@Component
struct Index {
  @State message: string = 'Hello World'
  @State nativeStr: string = ""
  @State pjsipVersion : string = "unknow"

  @State counter : number = 0
  @State codecs : string = ""

  inputAccount:string = "105"
  inputPassword:string = "105"

  observer:SipObserver = {
    onRegStateChanged: (code: number, msg?: string): void => {
      console.log("onRegStateChanged code " + code +"  " + msg)
    },
    onIncomingCall: (callId: string, info?: string): void => {
      console.log("onIncomingCall callId " + callId)
    },
    onStartAudio: (callId: string): void => {
      console.log("onStartAudio callId " + callId)
    },
    onDisconnect: (callId: string): void => {
      console.log("native_log onDisconnect callId " + callId)
    }
  }

  aboutToAppear(): void {
    this.nativeStr = NativeBridge.getNativeString()
    this.pjsipVersion = NativeBridge.getPjsipVersionStr()
    this.codecs = NativeBridge.findCodecFormats()

    this.registerObserver(this.observer)
  }

  aboutToDisappear(): void {
    this.unRegisterObserver(this.observer)
  }

  registerObserver(observer:SipObserver):void {

  }

  unRegisterObserver(observer:SipObserver):void{

  }

  build() {
    Row() {
      Column() {
        Text("TestFunc").fontSize(30)
          .onClick((v)=>{
            NativeBridge.RegisterSipObserver(this.observer)
          })

        Text("PjsipVersion:" + this.pjsipVersion)
          .fontSize($r('app.float.page_text_font_size'))
          .fontWeight(FontWeight.Bold)

        // Text(this.message)
        //   .fontSize($r('app.float.page_text_font_size'))
        //   .fontWeight(FontWeight.Bold)
        //   .onClick(() => {
        //     this.message = NativeBridge.getNativeString()
        //   })

        // Text(this.codecs)
        //   .fontSize(10)
        //
        // Button(this.counter.toString())
        //   .type(ButtonType.Circle)
        //   .width(100)
        //   .onClick(()=>{
        //     this.testButtonClick()
        //   })

        TextInput({
          placeholder:"Sip账号",
          text:this.inputAccount
        }).fontSize(20)
          .margin({top:8})
          .onChange((v)=>{
            this.inputAccount = v
          })

        TextInput({
          placeholder:"Sip密码",
          text:this.inputPassword
        }).fontSize(20)
          .margin({top:8})
          .onChange((v)=>{
            this.inputPassword = v
          })

        Button("Sip登录")
          .margin({top:8})
          .onClick(()=>{
            NativeBridge.SipLogin(this.inputAccount, this.inputPassword)
          })

        // Button("Sip登录")
        //   .onClick(()=>{
        //     this.sipLogin()
        //   })
      }
      .width('100%')
    }
    .height('100%')
  }

  testButtonClick(){
    NativeBridge.registerCallback2( this.counter, (v:number)=>{
      this.counter = v
    })
  }

  sipLogin():void{
  }
}
