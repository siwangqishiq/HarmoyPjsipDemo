import { hilog } from '@kit.PerformanceAnalysisKit';
import NativeBridge from 'libyxpjsip.so';
import { CallPageParams } from './CallPage';
import { SipObserver } from './SipObserver';

@Entry
@Component
struct Index {
  pathStack:NavPathStack = new NavPathStack()

  @State message: string = 'Hello World'
  @State nativeStr: string = ""
  @State pjsipVersion : string = "unknow"

  @State counter : number = 0
  @State codecs : string = ""

  inputAccount:string = "105"
  inputPassword:string = "105"

  input_make_call_number:string = ""

  observer:SipObserver = {
    onRegStateChanged: (code: number, msg?: string): void => {
      console.log("onRegStateChanged code " + code +"  " + msg)
      this.message = "onRegStateChanged code: " + code +"  reason" + msg??""
    },
    onIncomingCall: (callId: string, info?: string): void => {
      console.log("onIncomingCall callId " + callId)
      const params:CallPageParams = {
        isCaller:false,
        callId:callId
      }
      this.pathStack?.pushPath({
        name:"CallPage",
        param:params
      });
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

    NativeBridge.RegisterSipObserver(this.observer)
  }

  aboutToDisappear(): void {
    NativeBridge.UnregisterSipObserver(this.observer)
  }

  build() {
    Navigation(this.pathStack){
      Row() {
        Column() {
          Text(this.message).fontSize(30)
            .onClick((v)=>{
              NativeBridge.RegisterSipObserver(this.observer)
              this.message = ""
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

          TextInput({
            placeholder:"拨打号码",
            text:this.input_make_call_number
          }).fontSize(20)
            .margin({top:32})
            .onChange((v)=>{
              this.input_make_call_number = v
            })

          Button("拨打电话")
            .margin({top:8})
            .onClick(()=>{
              const call_id = NativeBridge.SipMakeCall(this.input_make_call_number)

              console.log("MakeCall callId " + call_id)
              const params:CallPageParams = {
                isCaller:true,
                callId:call_id
              }
              this.pathStack?.pushPath({
                name:"CallPage",
                param:params
              });
            })

          // Button("点击测试")
          //   .onClick(()=>{
          //
          //     // this.pathStack?.pushPath({
          //     //   name:"CallPage",
          //     // })
          //   })
        }
        .width('100%')
      }
      .height('100%')
    }
  }

  testButtonClick(){
    NativeBridge.registerCallback2( this.counter, (v:number)=>{
      this.counter = v
    })
  }
}
