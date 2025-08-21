import { hilog } from '@kit.PerformanceAnalysisKit';
import NativeBridge from 'libentry.so';

const DOMAIN = 0x0000;

@Entry
@Component
struct Index {
  @State message: string = 'Hello World';
  @State nativeStr: string = "";
  @State pjsipVersion : string = "unknow";

  @State counter : number = 0;

  aboutToAppear(): void {
    this.nativeStr = NativeBridge.getNativeString()
    this.pjsipVersion = NativeBridge.getPjsipVersionStr()
  }

  build() {
    Row() {
      Column() {
        Text("PjsipVersion:" + this.pjsipVersion)
          .fontSize($r('app.float.page_text_font_size'))
          .fontWeight(FontWeight.Bold)

        Text(this.message)
          .fontSize($r('app.float.page_text_font_size'))
          .fontWeight(FontWeight.Bold)
          .onClick(() => {
            this.message = NativeBridge.getNativeString()
          })

        Button(this.counter.toString())
          .type(ButtonType.Circle)
          .width(100)
          .onClick(()=>{
            this.testButtonClick()
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
