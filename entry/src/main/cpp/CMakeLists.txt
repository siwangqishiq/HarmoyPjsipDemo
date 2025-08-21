# the minimum version of CMake.
cmake_minimum_required(VERSION 3.5.0)
project(HarmoyPjsipDem)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

message(${NATIVERENDER_ROOT_PATH})

find_library(hilog-lib libhilog_ndk.z.so)

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)

add_library(entry SHARED
               napi_init.cpp)

target_link_libraries(entry PUBLIC ${hilog-lib} libace_napi.z.so)

# 将三方库的头文件加入工程中
target_include_directories(entry PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pjsip/${OHOS_ARCH}/include/)

# 设置对应的配置项
target_compile_definitions(entry PUBLIC -DPJ_AUTOCONF=1  -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1)

# 将三方库加入工程中
target_link_directories(entry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pjsip/${OHOS_ARCH}/lib ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/opus/${OHOS_ARCH}/lib)
if(${OHOS_ARCH} STREQUAL "arm64-v8a")
    message("arm64-v8a Architecture")
    target_link_libraries(entry PUBLIC
        pjsua2-aarch64-unknown-linux-gnu
        pjsua-aarch64-unknown-linux-gnu
        pjsip-ua-aarch64-unknown-linux-gnu
        pjsip-simple-aarch64-unknown-linux-gnu
        pjsip-aarch64-unknown-linux-gnu
        pjmedia-codec-aarch64-unknown-linux-gnu
        pjmedia-aarch64-unknown-linux-gnu
        pjmedia-videodev-aarch64-unknown-linux-gnu
        pjmedia-audiodev-aarch64-unknown-linux-gnu
        pjmedia-aarch64-unknown-linux-gnu
        pjnath-aarch64-unknown-linux-gnu
        pjlib-util-aarch64-unknown-linux-gnu
        pj-aarch64-unknown-linux-gnu
        srtp-aarch64-unknown-linux-gnu
        resample-aarch64-unknown-linux-gnu
        gsmcodec-aarch64-unknown-linux-gnu
        speex-aarch64-unknown-linux-gnu
        ilbccodec-aarch64-unknown-linux-gnu
        g7221codec-aarch64-unknown-linux-gnu
        yuv-aarch64-unknown-linux-gnu  opus)
elseif(${OHOS_ARCH} STREQUAL "armeabi-v7a")
    target_link_libraries(entry PUBLIC
        pjsua2-arm-unknown-linux-gnu
        pjsua-arm-unknown-linux-gnu
        pjsip-ua-arm-unknown-linux-gnu
        pjsip-simple-arm-unknown-linux-gnu
        pjsip-arm-unknown-linux-gnu
        pjmedia-codec-arm-unknown-linux-gnu
        pjmedia-arm-unknown-linux-gnu
        pjmedia-videodev-arm-unknown-linux-gnu
        pjmedia-audiodev-arm-unknown-linux-gnu
        pjmedia-arm-unknown-linux-gnu
        pjnath-arm-unknown-linux-gnu
        pjlib-util-arm-unknown-linux-gnu
        pj-arm-unknown-linux-gnu
        srtp-arm-unknown-linux-gnu
        resample-arm-unknown-linux-gnu
        gsmcodec-arm-unknown-linux-gnu
        speex-arm-unknown-linux-gnu
        ilbccodec-arm-unknown-linux-gnu
        g7221codec-arm-unknown-linux-gnu
        yuv-arm-unknown-linux-gnu  opus)
elseif(${OHOS_ARCH} STREQUAL "x86_64")
    message("x86_64 Architecture")
    target_link_libraries(entry PUBLIC
            pjsua2-x86_64-pc-linux-gnu
            pjsua-x86_64-pc-linux-gnu
            pjsip-ua-x86_64-pc-linux-gnu
            pjsip-simple-x86_64-pc-linux-gnu
            pjsip-x86_64-pc-linux-gnu
            pjmedia-codec-x86_64-pc-linux-gnu
            pjmedia-x86_64-pc-linux-gnu
            pjmedia-videodev-x86_64-pc-linux-gnu
            pjmedia-audiodev-x86_64-pc-linux-gnu
            pjmedia-x86_64-pc-linux-gnu
            pjnath-x86_64-pc-linux-gnu
            pjlib-util-x86_64-pc-linux-gnu
            pj-x86_64-pc-linux-gnu
            srtp-x86_64-pc-linux-gnu
            resample-x86_64-pc-linux-gnu
            gsmcodec-x86_64-pc-linux-gnu
            speex-x86_64-pc-linux-gnu
            ilbccodec-x86_64-pc-linux-gnu
            g7221codec-x86_64-pc-linux-gnu
            yuv-x86_64-pc-linux-gnu  opus)
else()
  message("not support Architecture")
endif()