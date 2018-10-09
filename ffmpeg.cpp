#include "ffmpeg.h"
#include <qdebug>
#include <QDebug>
#include <QTime>
#include <QApplication>
#include "colorplayer.h"

#define SAVEYUV 0
extern PlayerInfo *pVS;


static int getTimeInUs()
{
    QTime current_time =QTime::currentTime();
    int hour = current_time.hour();//当前的小时
    int minute = current_time.minute();//当前的分
    int second = current_time.second();//当前的秒
    int msec = current_time.msec();//当前的毫秒

    return (hour * 60 * 60 * 1000000 + minute * 60 * 1000000 + second * 1000000 + msec * 1000);
}

static void sleepMs_my(unsigned int SleepMs)
{
    QTime dieTime = QTime::currentTime().addMSecs(SleepMs);
    while(QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

static double r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

bool XFFmpeg::Open(const char *path)
{
    mutex.lock();

#if SAVEYUV
    fp = fopen("test.yuv","w+");
    if (fp == NULL)
    {
        mutex.unlock();
        printf("open test.yuv error !");
        return false;
    }
    qDebug()<<"test.yuv ok";
#endif

    int ret = avformat_open_input(&ic, path, NULL, NULL);
    if (ret != 0 || ic == NULL)
    {
        mutex.unlock();
        av_strerror(ret, errorbuf, sizeof(errorbuf));
        qDebug()<<"avformat_open_input error";
        return false;
    }

    if(avformat_find_stream_info(ic,NULL)<0){
        qDebug()<<"Couldn't find stream information.\n";
        return false;
    }

    if (ic->duration != AV_NOPTS_VALUE)
    {
        int64_t duration = ic->duration + (ic->duration <= INT64_MAX - 5000 ? 5000 : 0);
        totalMs = (duration / AV_TIME_BASE) * 1000;
    }

    qDebug()<<"totalMs = "<<totalMs;

    for (unsigned int i = 0; i < ic->nb_streams; i++)
    {
        AVCodecContext *codecCTX_tmp = ic->streams[i]->codec;
        if ((codecCTX_tmp->codec_type == AVMEDIA_TYPE_VIDEO) && (videostreamidx == -1))//视频流
        {
            videostreamidx = i;
            video_avctx = ic->streams[i]->codec;
            qDebug()<<"videostreamidx "<<videostreamidx;
            fps = r2d(ic->streams[i]->r_frame_rate);
            qDebug()<<"ffmpeg video r_frame_rate:"<<r2d(ic->streams[i]->r_frame_rate);
            AVCodec *codec = avcodec_find_decoder(codecCTX_tmp->codec_id);
            if (!codec)
            {
                mutex.unlock();
                qDebug()<<"not find decoder";
                return false;
            }
            else
            {
                int ret = avcodec_open2(codecCTX_tmp, codec, NULL);
                if (ret != 0)
                {
                    mutex.unlock();
                    char buff[1024] = { 0 };
                    av_strerror(ret, buff, sizeof(buff));
                    qDebug()<<"avcodec_open2 err";
                    return false;
                }
                width  = codecCTX_tmp->width;
                height = codecCTX_tmp->height;
                qDebug()<<"find vdieo dec "<<i<<"width ="<<width<<"height"<<height;
            }
        }
        else if ((codecCTX_tmp->codec_type == AVMEDIA_TYPE_AUDIO) && (audioStreamidx == -1))//音频流
        {
            audioStreamidx = i;
            qDebug()<<"audio duration==>"<<ic->streams[i]->duration<<"frames "<<ic->streams[i]->codec->frame_number;
            audio_avctx = ic->streams[i]->codec;
            qDebug()<<"audioStreamidx "<<audioStreamidx;
            AVCodec *codec = avcodec_find_decoder(codecCTX_tmp->codec_id);
            if (codec == NULL || (avcodec_open2(codecCTX_tmp, codec, NULL) < 0))
            {
                qDebug()<<"audio open failed";
                mutex.unlock();
                return false;
            }
            this->sampleRate = codecCTX_tmp->sample_rate;
            this->channel = codecCTX_tmp->channels;
            this->frame_size = codecCTX_tmp->frame_size;

            switch (codecCTX_tmp->sample_fmt)
            {
            case AV_SAMPLE_FMT_S16:
                this->sampleSize = 16;
                break;
            case AV_SAMPLE_FMT_S32:
                this->sampleSize = 32;
            default:
                break;
            }
            qDebug()<<"ffmpeg: sampleRate "<<codecCTX_tmp->sample_rate;
            qDebug()<<"ffmpeg: channel "<<codecCTX_tmp->channels;
            qDebug()<<"ffmpeg: frame_size "<<codecCTX_tmp->frame_size;
            qDebug()<<"ffmpeg: sampleSize "<<this->sampleSize;
        }
    }

    mutex.unlock();
    return true;
}

void XFFmpeg::Close()
{
    mutex.lock();

    qDebug()<<"XFFmpeg::Close()";
#if SAVEYUV
    if (fp)
    {
        fclose(fp);
    }
#endif

    if (SwsCtx)
    {
        sws_freeContext(SwsCtx);
        SwsCtx = NULL;
    }

    if (SwrCtx)
    {
        swr_free(&SwrCtx);
    }

    if (ic)
    {
        avformat_close_input(&ic);
        ic = NULL;
    }

    videostreamidx = -1;
    audioStreamidx = -1;

    mutex.unlock();
}

std::string XFFmpeg::GetError()
{
    mutex.lock();
    std::string re = this->errorbuf;
    mutex.unlock();
    return re;
}

AVPacket XFFmpeg::Read()
{
    AVPacket pkt;
    memset(&pkt, 0, sizeof(AVPacket));
    mutex.lock();
    if (!ic)
    {
        mutex.unlock();
        return pkt;
    }
    int err = av_read_frame(ic, &pkt);
    if (err != 0)
    {
        av_strerror(err, errorbuf, sizeof(errorbuf));
    }
    mutex.unlock();
    return pkt;
}

int XFFmpeg::GetPts(const AVPacket *pkt)
{
    mutex.lock();
    if (!ic)
    {
        mutex.unlock();
        return -1;
    }
    int pts = pkt->pts * r2d(ic->streams[pkt->stream_index]->time_base) * 1000;
    mutex.unlock();

    return pts;
}

void XFFmpeg::Flush()
{
    avcodec_flush_buffers(video_avctx);
    avcodec_flush_buffers(audio_avctx);
}

int XFFmpeg::Decode(const AVPacket *pkt, AVFrame *frame)
{
    mutex.lock();

    if (!ic || pkt == NULL || frame == NULL)
    {
        qDebug()<<"err 0";
        mutex.unlock();
        return NULL;
    }

    if (pkt->stream_index != videostreamidx && pkt->stream_index != audioStreamidx)
    {
        //qDebug()<<"pkt->stream_index error "<<pkt->stream_index;
        qDebug()<<"videostreamidx "<<videostreamidx<<"audioStreamidx "<<audioStreamidx;
        mutex.unlock();
        return NULL;
    }

    if (avcodec_send_packet(ic->streams[pkt->stream_index]->codec, pkt) != 0)
    {
        qDebug()<<"err avcodec_send_packet";
        mutex.unlock();
        return NULL;
    }

    if (avcodec_receive_frame(ic->streams[pkt->stream_index]->codec, frame) != 0)
    {
        qDebug()<<"err avcodec_receive_frame";
        mutex.unlock();
        return NULL;
    }

    AVRational playTimeBase;
    playTimeBase.num = 1;
    playTimeBase.den = 1000;
    //qDebug()<<"stream_index "<<pkt->stream_index<<" before timebase convert pts:"<<frame->pts;
    frame->pts = av_rescale_q_rnd(frame->pts,ic->streams[pkt->stream_index]->time_base,
            ic->streams[pkt->stream_index]->codec->time_base, AV_ROUND_NEAR_INF);
    frame->pts = av_rescale_q_rnd(frame->pts,ic->streams[pkt->stream_index]->codec->time_base,
            playTimeBase, AV_ROUND_NEAR_INF);
    //qDebug()<<"stream_index "<<pkt->stream_index<<" after timebase convert pts:"<<frame->pts;

#if SAVEYUV
    if (pkt->stream_index == videostreamidx)
    {
        //printf("aaaaaaaaaaaaaa\n");
        qDebug()<<"width"<<ic->streams[pkt->stream_index]->codec->width<<"height "<<ic->streams[pkt->stream_index]->codec->height;
        int y_size = ic->streams[pkt->stream_index]->codec->width * ic->streams[pkt->stream_index]->codec->height;
        fwrite(frame->data[0], 1, y_size, fp);      //Y
        fwrite(frame->data[1], 1, y_size / 4, fp);  //U
        fwrite(frame->data[2], 1, y_size / 4, fp);  //V
    }
#endif

    mutex.unlock();

    return 1;
}

int XFFmpeg::PutFrameToConvert(int StreamID, AVFrame *pFrame)
{
    mutex.lock();

    if (StreamID == audioStreamidx && pFrame != NULL)
    {
        pcm = pFrame;
    }
    else if (StreamID == videostreamidx && pFrame != NULL)
    {
        yuv = pFrame;
    }
    else
    {
        qDebug()<<"error streamID";
        mutex.unlock();
        return -1;
    }

    mutex.unlock();
    return 0;
}

bool XFFmpeg::ToRGB(char *out, int outwidth, int outheight)
{
    mutex.lock();
    if (!ic || yuv == NULL)
    {
        mutex.unlock();
        return false;
    }

    //qDebug()<<"ToRGB pts = "<<yuv->pts;
    AVCodecContext *codecCTX = ic->streams[this->videostreamidx]->codec;
    if (codecCTX->pix_fmt == AV_PIX_FMT_NONE)
    {
        codecCTX->pix_fmt = AV_PIX_FMT_YUV420P;//如果发现解封装后无pix_fmt，将强制为420
    }
    SwsCtx = sws_getCachedContext(SwsCtx, codecCTX->width, codecCTX->height, codecCTX->pix_fmt,
        outwidth, outheight, AV_PIX_FMT_BGRA, SWS_BICUBIC, NULL, NULL, NULL);
    if (!SwsCtx)
    {
        mutex.unlock();
        yuv = NULL;
        printf("sws_getCachedContext error\n");
        return false;
    }
    uint8_t *data[AV_NUM_DATA_POINTERS];
    data[0] = (uint8_t *)out;
    int linesize[AV_NUM_DATA_POINTERS];
    linesize[0] = outwidth * 4;
    int height = sws_scale(SwsCtx, yuv->data, yuv->linesize, 0, yuv->height,
        data, linesize);
    if (height < 0)
    {
        yuv = NULL;
        mutex.unlock();
        return true;
        printf("sws_scale error\n" );
    }

    yuv = NULL;
    mutex.unlock();
    return true;
}

int XFFmpeg::ToPCM(char *out)
{
    mutex.lock();
    if (!ic || !pcm || !out)
    {
        mutex.unlock();
        return 0;
    }
    AVCodecContext *ctx = ic->streams[audioStreamidx]->codec;

    if (SwrCtx == NULL)
    {
        SwrCtx = swr_alloc();
        swr_alloc_set_opts(SwrCtx, ctx->channel_layout, AV_SAMPLE_FMT_S16,
            ctx->sample_rate, ctx->channels, ctx->sample_fmt, ctx->sample_rate,
            0, 0);
        swr_init(SwrCtx);
    }

    uint8_t *data[1];
    data[0] = (uint8_t *)out;
    int len = swr_convert(SwrCtx, data, 10000, (const uint8_t **)pcm->data, pcm->nb_samples);
    if (len <= 0)
    {
        mutex.unlock();
        return 0;
    }
    int outsize = av_samples_get_buffer_size(NULL, ctx->channels,
        pcm->nb_samples, AV_SAMPLE_FMT_S16, 0);
    mutex.unlock();
    return outsize;
}

/*pos 表示 0~100%*/
bool XFFmpeg::Seek(float pos)
{
    int ret = -1;

    mutex.lock();
    if (!ic)
    {
        mutex.unlock();
        return false;
    }

    seek_stamp = (pos * totalMs / 1000) * AV_TIME_BASE;//ic->streams[videostreamidx]->duration;
    qDebug()<<"pos :"<<pos<<"seek_stamp(us):"<<seek_stamp<<"totalMs :"<<totalMs;
    XFFmpeg::Get()->bSeek = 1;

    ret = avformat_seek_file(ic, -1, INT64_MIN, seek_stamp, INT64_MAX, AVSEEK_FLAG_BACKWARD);
    mutex.unlock();

    return (ret >= 0 ? true : false);
}

XFFmpeg::XFFmpeg()
{
    errorbuf[0] = '\0';
    av_register_all();
    videostreamidx = -1;
    audioStreamidx = -1;
}


XFFmpeg::~XFFmpeg()
{
    //XFFmpeg::Close();
    if (SwsCtx)
    {
        sws_freeContext(SwsCtx);
        SwsCtx = NULL;
    }

    if (SwrCtx)
    {
        swr_free(&SwrCtx);
    }

    if (ic)
    {
        avformat_close_input(&ic);
        ic = NULL;
    }
}
