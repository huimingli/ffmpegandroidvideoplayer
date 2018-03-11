/*
    SDL_android_main.c, placed in the public domain by Sam Lantinga  3/13/14
*/
#include "../../SDL_internal.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

#ifdef __ANDROID__

/* Include the SDL main definition header */
#include "SDL_main.h"


/*******************************************************************************
                 Functions called by JNI
*******************************************************************************/
#include <jni.h>
#include <android/log.h>

#include "org_libsdl_app_SDLActivity.h"
#include "../../../../Media/Media.h"
#include "../../../../ReadPacketsThread/ReadPacketsThread.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include <libavformat/avformat.h>
#include "libswscale/swscale.h"
#include <libavutil/time.h>
#include "libavutil/imgutils.h"
}
/* Called before SDL_main() to initialize JNI bindings in SDL library */
extern "C"
extern void SDL_Android_Init(JNIEnv *env, jclass cls);

std::string jstring2str(JNIEnv* env1, jstring jstr) {
    char*   rtn   =   NULL;

    jclass   clsstring   =   env1->FindClass("java/lang/String");
    jstring   strencode   =   env1->NewStringUTF("GB2312");
    jmethodID   mid   =   env1->GetMethodID(clsstring,   "getBytes",   "(Ljava/lang/String;)[B");
    jbyteArray   barr=   (jbyteArray)env1->CallObjectMethod(jstr,mid,strencode);
    jsize   alen   =   env1->GetArrayLength(barr);
    jbyte*   ba   =   env1->GetByteArrayElements(barr,JNI_FALSE);
    if(alen   >   0)
    {
        rtn   =   (char*)malloc(alen+1);
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
    }
    env1->ReleaseByteArrayElements(barr,ba,0);
    std::string stemp(rtn);
    free(rtn);
    return   stemp;
}
/* This prototype is needed to prevent a warning about the missing prototype for global function below */

/* Start up the SDL app */
JNIEXPORT jint JNICALL
Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv *env, jclass cls, jobject surface) {
    int i;
    int argc;
    int status;
    int len;
    char *argv[3];

    /* This interface could expand with ABI negotiation, callbacks, etc. */
    SDL_Android_Init(env, cls);

    SDL_SetMainReady();

    /* Prepare the arguments. */

    argv[0] = SDL_strdup("app_process");

    status = SDL_main(argc, argv);


    for (i = 0; i < argc; ++i) {
        SDL_free(argv[i]);
    }
    SDL_stack_free(argv);
    /* Do not issue an exit or the whole application will terminate instead of just the SDL thread */
    /* exit(status); */

    return status;
}

static bool isPlay = true;
JNIEXPORT jint JNICALL
Java_org_libsdl_app_SDLActivity_nativePauseVideo(JNIEnv *env, jclass cls){

    isPlay = !isPlay;
    if (isPlay)
    {
        ReadPacketsThread::getInstance()->setPlaying(true);
        if(Media::getInstance()->getAVFormatContext())
            SDL_PauseAudio(0);
        return 1;
    }
    else
    {
        ReadPacketsThread::getInstance()->setPlaying(false);
        if (Media::getInstance()->getAVFormatContext())
            SDL_PauseAudio(1);
        return 0;
    }

}

JNIEXPORT jdouble JNICALL
Java_org_libsdl_app_SDLActivity_nativeGetPts(JNIEnv *env, jclass cls){

    return (double)Media::getInstance()->audio->pts * 1000;
}

JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_nativeQuit
        (JNIEnv *, jclass){
    __android_log_print(ANDROID_LOG_DEBUG, "DEBUG", "ma nativequit");
}

JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_nativePause
        (JNIEnv *, jclass){
    __android_log_print(ANDROID_LOG_DEBUG, "DEBUG", "ma nativePause");
}

JNIEXPORT jdouble JNICALL
Java_org_libsdl_app_SDLActivity_nativeGetTotalMs(JNIEnv *env, jclass cls){
    return (double)Media::getInstance()->totalMs;
}

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_nativeSeekVideo(JNIEnv *env, jclass cls,jfloat pos){
    ReadPacketsThread::getInstance()->receivePos(pos);
}
JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_nativeSelectFile
        (JNIEnv *env, jclass clx, jstring filename){
    __android_log_print(ANDROID_LOG_DEBUG, "DEBUG", "Java_org_libsdl_app_SDLActivity_nativeSelectFile");
    Media::getInstance()
            ->setMediaFile(jstring2str(env,filename).c_str())
            ->config();
    ReadPacketsThread::getInstance()->setPlaying(true);
}
static const double SYNC_THRESHOLD = 0.01;//同步临界值
static const double NOSYNC_THRESHOLD = 10.0;//非同步临界值
JNIEXPORT jint JNICALL
Java_org_libsdl_app_SDLActivity_nativePlay(JNIEnv *env, jclass cls, jobject surface) {

    if(!isPlay){
        return  1;
    }
    if (Media::getInstance()->video->getVideoFrameSiez() <= 0)
        return 10;
    AVFormatContext *pFormatCtx = Media::getInstance()->getAVFormatContext();
    if (pFormatCtx == NULL){
        return 1;
    }
    // Get a pointer to the codec context for the video stream
    AVCodecContext *pCodecCtx = pFormatCtx->streams[Media::getInstance()->getVideoStreamIndex()]->codec;


    // 获取native window

    if(surface == NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, "DEBUG", "empty surface");
        return 1;
    }
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 获取视频宽高
    int videoWidth = pCodecCtx->width;
    int videoHeight = pCodecCtx->height;

    __android_log_print(ANDROID_LOG_DEBUG, "DEBUG", "video width%d height%d ", videoWidth,
                        videoHeight);


    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;




    // 用于渲染
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL  ) {
        return 10;
    }

    // Determine required buffer size and allocate buffer
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height,
                                            1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         pCodecCtx->width, pCodecCtx->height, 1);

    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,
                                                pCodecCtx->height,
                                                pCodecCtx->pix_fmt,
                                                pCodecCtx->width,
                                                pCodecCtx->height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);


    // Is this a packet from the video stream?
    // Decode video frame

    AVFrame *pFrame = Media::getInstance()->video->dequeueFrame();

    // 将视频同步到音频上，计算下一帧的延迟时间
    double current_pts = *(double*)pFrame->opaque;
    double delay = current_pts - Media::getInstance()->video->getFrameLastPts();
    if (delay <= 0 || delay >= 1.0)
        delay = Media::getInstance()->video->getFrameLastDelay();
    Media::getInstance()->video->setFrameLastDelay(delay);
    Media::getInstance()->video->setFrameLastPts(current_pts);
    // 当前显示帧的PTS来计算显示下一帧的延迟
    double ref_clock = Media::getInstance()->audio->getAudioClock();
    double diff = current_pts - ref_clock;// diff < 0 => video slow,diff > 0 => video quick
    double threshold = (delay > SYNC_THRESHOLD) ? delay : SYNC_THRESHOLD;
    if (fabs(diff) < NOSYNC_THRESHOLD) // 不同步
    {
        if (diff <= -threshold) // 慢了，delay设为0
            delay = 0;
        else if (diff >= threshold) // 快了，加倍delay
            delay *= 2;
    }
    Media::getInstance()->video->setFrameTimer(Media::getInstance()->video->getFrameTimer() + delay) ;

    double actual_delay = Media::getInstance()->video->getFrameTimer() - static_cast<double>(av_gettime()) / 1000000.0;
    if (actual_delay <= 0.010)
        actual_delay = 0.010;

    // 并不是decode一次就可解码出一帧
    // lock native window buffer
    ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

    // 格式转换
    sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
              pFrame->linesize, 0, pCodecCtx->height,
              pFrameRGBA->data, pFrameRGBA->linesize);

    // 获取stride
    uint8_t *dst = (uint8_t *) windowBuffer.bits;
    int dstStride = windowBuffer.stride * 4;
    uint8_t *src = (uint8_t *) (pFrameRGBA->data[0]);
    int srcStride = pFrameRGBA->linesize[0];

    // 由于window的stride和帧的stride不同,因此需要逐行复制
    int h;
    for (h = 0; h < videoHeight; h++) {
        memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(nativeWindow);
    av_free(buffer);
    buffer = nullptr;
    av_frame_unref(pFrameRGBA);
    av_frame_free(&pFrameRGBA);
    pFrameRGBA = nullptr;
    // Free the YUV frame
    av_frame_unref(pFrame);
    av_frame_free(&pFrame);
    pFrame = nullptr;
    sws_freeContext(sws_ctx);
    return static_cast<int>(actual_delay * 1000 + 0.5);
}




#endif /* __ANDROID__ */

/* vi: set ts=4 sw=4 expandtab: */
