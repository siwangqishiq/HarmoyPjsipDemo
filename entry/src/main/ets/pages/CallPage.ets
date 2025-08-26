import NativeBridge from 'libyxpjsip.so';
import { SipObserver } from './SipObserver';

export interface CallPageParams{
  isCaller:boolean
}

@Builder
export function CallPageBuilder() {
  CallPage()
}

@Component
export struct CallPage {
  pathStack?: NavPathStack
  params?:CallPageParams

  observer:SipObserver = {
    onRegStateChanged: (code: number, msg?: string): void => {
      console.log("CallPage onRegStateChanged code " + code +"  " + msg)
    },
    onIncomingCall: (callId: string, info?: string): void => {
      console.log("CallPage onIncomingCall callId " + callId)
    },
    onStartAudio: (callId: string): void => {
      console.log("CallPage onStartAudio callId " + callId)
    },
    onDisconnect: (callId: string): void => {
      console.log("CallPage native_log onDisconnect callId " + callId)
    }
  }

  aboutToAppear(): void {
    NativeBridge.RegisterSipObserver(this.observer)
  }

  aboutToDisappear(): void {
    NativeBridge.UnregisterSipObserver(this.observer)
  }

  build() {
    NavDestination(){
      Row() {
        Column() {
          Text("CallPage").fontSize(50)
            .margin({top:8})
            .onClick((v)=>{
              NativeBridge.RegisterSipObserver(this.observer)
            })

          Button("接通")
            .margin({top:8})
          Button("挂断")
            .margin({top:8})
            .onClick(()=>{
              this.pathStack?.pop()
            })
        }
        .width('100%')
      }
      .height('100%')
    }.hideTitleBar(true)
    .backgroundColor(Color.White)
    .onReady((context: NavDestinationContext) => {
      this.pathStack = context.pathStack
    })
  }
}
