#ifndef FFMPEG_H
#define FFMPEG_H

extern "C"{
#include<libavformat\avformat.h>
#include<libswscale/swscale.h>
#include<libswresample/swresample.h>
#include<libavutil/rational.h>
}

#include<string>
#include<QMutex>


class XFFmpeg
{
public:
    static XFFmpeg *Get()
    {
        static XFFmpeg ff;
        return &ff;
    }
    /////////////////////////////////////////////
    ///注释
    bool Open(const char *path);
    void Close();
    AVPacket Read(int *pEof);
    int Decode(const AVPacket *pkt, AVFrame *frame);//返回pts
    void Flush();
    int GetPts(const AVPacket *pkt);//返回码流包的pts
    int IsOnlyMusic(void);//是否只有音频流
    int IsOnlyVideo(void);//是否只有视频流
    bool ToRGB(char *out, int outwidth, int outheight);
    int PutFrameToConvert(int StreamID, AVFrame *pFrame);
    int ToPCM(char *out);//转换为pcm格式
    /*0~100%*/
    bool Seek(float pos);
    bool bSeek;
    int64_t seek_stamp;
    std::string GetError();
    virtual ~XFFmpeg();
    int totalMs = 0;
    int fps = 0;
    int pts = 0;//
    int videostreamidx = -1;
    int audioStreamidx = -1;
    int width = 380;
    int height = 240;
    int sampleRate = 0;
    int sampleSize = 16;
    int sampleFormate = 1;
    int channel = 0;
    int frame_size = 0;
    int nb_samples = 1024;
    bool isPlay = true;
    FILE *fp = NULL;
    AVFormatContext *ic = NULL;
    AVCodecContext *video_avctx = NULL;
    AVCodecContext *audio_avctx = NULL;
    bool ReOpen = 0;
protected:
    char errorbuf[1024];
    QMutex mutex;
    QMutex mutex_putConvert;
    AVFrame *yuv = NULL;
    AVFrame *pcm = NULL;
    SwsContext *SwsCtx = NULL;
    SwrContext *SwrCtx = NULL;//音频

    XFFmpeg();
};


#endif // FFMPEG_H
