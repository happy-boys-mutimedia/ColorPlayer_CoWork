#include "videoThread.h"
#include "ffmpeg.h"
#include "AudioPlay.h"
#include <list>
#include <qdebug>
#include "readfilethread.h"

using namespace std;
bool isexit = false;

static list<AVPacket> videos;

XVideoThread::XVideoThread()
{
}

void XVideoThread::run()
{
    int audioPts = -1;
    int videoPts = -1;
    char out[15000] = { 0 };

    while (!isexit)
    {
        if (!XFFmpeg::Get()->isPlay)
        {
            msleep(10);
            continue;
        }

        /*while (videos.size() > 0)
        {
            AVPacket pack = videos.front();
            int pts = XFFmpeg::Get()->GetPts(&pack);
            qDebug()<<pts;
            qDebug()<<audioPts;
            if (pts > audioPts)//视频比音频pts小的时候启动视频帧解码
            {
                qDebug()<<pts;
                qDebug()<<audioPts;
                break;
            }
            XFFmpeg::Get()->Decode(&pack);
            av_packet_unref(&pack);
            videos.pop_front();
        }*/

        /*int free = AudioPlay::Get()->GetFree();//音频剩余空间
        if (free < 15000)
        {
            msleep(1);
            continue;
        }*/

        AVPacket pkt = XFFmpeg::Get()->Read();
        if (pkt.size <= 0)
        {
            msleep(10);
            continue;
        }

        if (pkt.stream_index == XFFmpeg::Get()->audioStreamidx)
        {
            audioPts = XFFmpeg::Get()->Decode(&pkt);
            qDebug()<<"audioPts";
            qDebug()<<audioPts;
            av_packet_unref(&pkt);
            int len = XFFmpeg::Get()->ToPCM(out);
            AudioPlay::Get()->Write(out, len);
            continue;
        }

        /*if (pkt.stream_index != XFFmpeg::Get()->videostreamidx || pkt.size == 0)
        {
            av_packet_unref(&pkt);
            continue;
        }*/

        readfilethread::Get()->VideoDecode.wait();
        videoPts = XFFmpeg::Get()->Decode(&pkt);
        qDebug()<<videoPts;
        av_packet_unref(&pkt);
        //readfilethread::Get()->VideoDecode.wakeAll();

        //readfilethread::Get()->Readmutex.unlock();
        //qDebug()<<"videoPts";
        //qDebug()<<videoPts;
        //videos.push_back(pkt);//优化同步，让音频一直播放，视频去同步音频进行播放
        if (XFFmpeg::Get()->fps > 0)
        {
            msleep(1000);//(XFFmpeg::Get()->fps)
        }
    }
}

XVideoThread::~XVideoThread()
{
}
