import { hilog } from '@kit.PerformanceAnalysisKit';
import NativeBridge from 'libyxpjsip.so';

const DOMAIN = 0x0000;

@Entry
@Component
struct Index {
  @State message: string = 'Hello World'
  @State nativeStr: string = ""
  @State pjsipVersion : string = "unknow"

  @State counter : number = 0
  @State codecs : string = ""

  inputAccount:string = ""
  inputPassword:string = ""

  aboutToAppear(): void {
    this.nativeStr = NativeBridge.getNativeString()
    this.pjsipVersion = NativeBridge.getPjsipVersionStr()
    this.codecs = NativeBridge.findCodecFormats()
  }

  build() {
    Row() {
      Column() {
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
        }).fontSize(20)
          .margin({top:8})
          .onChange((v)=>{
            this.inputAccount = v
          })

        TextInput({
          placeholder:"Sip密码",
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
