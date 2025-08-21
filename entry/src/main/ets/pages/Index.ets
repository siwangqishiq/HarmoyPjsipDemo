import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';

const DOMAIN = 0x0000;

@Entry
@Component
struct Index {
  @State message: string = 'Hello World';
  @State nativeStr: string = "";

  @State pjsipVersion : string = "unknow";

  aboutToAppear(): void {
    this.nativeStr = testNapi.getNativeString()
    this.pjsipVersion = testNapi.getPjsipVersionStr()
  }

  build() {
    Row() {
      Column() {
        Text(this.message)
          .fontSize($r('app.float.page_text_font_size'))
          .fontWeight(FontWeight.Bold)
          .onClick(() => {
            this.message = testNapi.getNativeString()
          })

        Text("PjsipVersion:" + this.pjsipVersion)
          .fontSize($r('app.float.page_text_font_size'))
          .fontWeight(FontWeight.Bold)
      }
      .width('100%')
    }
    .height('100%')
  }
}
