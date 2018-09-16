#include "audiothread.h"
#include "ffmpeg.h"
#include "AudioPlay.h"
#include <list>
#include <qdebug>
#include "readfilethread.h"

using namespace std;

audiothread::audiothread()
{

}

void audiothread::run()
{
    char out[15000] = { 0 };
    qDebug()<<"start audio thread!";

    while(1)
    {
        /*if (!XFFmpeg::Get()->isPlay)
        {
            msleep(10);
            continue;
        }*/

        /*int free = AudioPlay::Get()->GetFree();//音频剩余空间
        if (free < 15000)
        {
            msleep(1);
            continue;
        }*/

        /*AVPacket pkt = XFFmpeg::Get()->Read();
        if (pkt.size <= 0)
        {
            msleep(10);
            continue;
        }*/
        AVPacket *pAVpkt = (AVPacket *)readfilethread::Get()->pAudioReadPkt;
        //readfilethread::Get()->AudioDecode.wait();
        //if (pkt.stream_index == XFFmpeg::Get()->audioStreamidx)
        if (pAVpkt != NULL)
        {
            int audioPts = XFFmpeg::Get()->Decode(pAVpkt);
            //qDebug()<<"audioPts";
            qDebug()<<"aaaa";
            av_packet_unref(pAVpkt);
            //readfilethread::Get()->AudioDecode.wakeAll();
            int len = XFFmpeg::Get()->ToPCM(out);
            AudioPlay::Get()->Write(out, len);
        }
    }
}

audiothread::~audiothread()
{

}
