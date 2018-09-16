#include "audiodecodethread.h"
#include "ffmpeg.h"
#include "QDebug"

AudioDecodeThread::AudioDecodeThread()
{
    pPlayerInfo = NULL;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"AudioDecodeThread() error\n";
    }
}

static Frame *GetOneValidFrame(FrameQueue *pQueue)
{
    Frame *pFrame = NULL;
    int i;

    if (pQueue == NULL)
    {
        qDebug()<<"pQueue == NULL";
        return NULL;
    }

    for (i = 0; i < pQueue->size; i++)
    {
        if (pQueue->queue[i].DecState != DecOver && pQueue->queue[i].DispState != DispWait
                && pQueue->queue[i].frame != NULL)
        {
            pFrame = &(pQueue->queue[i]);
            break;
        }
    }

    if (i == pQueue->size)
    {
        //qDebug()<<" can't find valid frame for dec";
        return NULL;
    }

    return pFrame;
}

static void FlashFrameQueue(FrameQueue *pQueue)
{
    int i;

    for (i = 0; i < pQueue->size; i++)
    {
        pQueue->queue[i].DecState = DecButt;
        pQueue->queue[i].DispState = DispButt;
    }

    return;
}

static myPacket *GetOnePacket(PacketQueue *pPacketQueue)
{
    myPacket *pTemp = NULL;

    if (pPacketQueue == NULL)
    {
        return NULL;
    }

    pPacketQueue->Mutex.lock();
    if (!pPacketQueue->Queue->isEmpty())
    {
        pTemp = pPacketQueue->Queue->takeFirst();
        pPacketQueue->Mutex.unlock();
        return pTemp;
    }
    pPacketQueue->Mutex.unlock();
    return NULL;
}

void AudioDecodeThread::run()
{
    int ret = -1;
    Frame *pFrame = NULL;
    AVPacket AvPkt = {0};
    myPacket MyPkt = {0};
    myPacket *pMyPkt = NULL;
    AVCodecContext *codecCTX = NULL;
    AVFormatContext *ictx = NULL;
    qDebug()<<"AudioDecodeThread in";

    while(1)
    {
        if ((pPlayerInfo == NULL) || !pPlayerInfo->isInitAll)
        {
            msleep(10);
            qDebug()<<"AudioDecodeThread error";
            continue;
        }

        if (pPlayerInfo->audioPacketQueue.Queue->isEmpty())
        {
            //qDebug()<<" audio Raw Queue empty !! :";
            msleep(10);
            continue;
        }

        if (!(pFrame = GetOneValidFrame(&pPlayerInfo->audioFrameQueue)))
        {
            //qDebug()<<"Audio GetOneValidFrame fail";
            msleep(10);
            continue;
        }

        if ((pMyPkt = GetOnePacket(&(pPlayerInfo->audioPacketQueue))) == NULL)
        {
            //qDebug()<<" Audio GetOnePacket fail";
            msleep(10);
            continue;
        }

        if (!XFFmpeg::Get()->ic)
        {
            msleep(10);
            continue;
        }
        codecCTX = XFFmpeg::Get()->ic->streams[XFFmpeg::Get()->audioStreamidx]->codec;
        //qDebug()<<"audio timebase num "<<codecCTX->time_base.num<<"den "<<codecCTX->time_base.den;
        //qDebug()<<"frame_size"<<codecCTX->frame_size<<"sample_rate"<<codecCTX->sample_rate;
#if 0//todo
        if (pMyPkt->AVPkt.data == flush_pkt.data && codecCTX != NULL)
        {
            pPlayerInfo->ADispQueue.mutex.lock();
            pPlayerInfo->ADispQueue.Queue->clear();
            pPlayerInfo->ADispQueue.mutex.unlock();
            FlashFrameQueue(&pPlayerInfo->audioFrameQueue);
            avcodec_flush_buffers(codecCTX);
            pPlayerInfo->bAudioFlash = 1;
            qDebug()<<"audio flash";
            continue;
        }
#endif
        if (pMyPkt->serial != pPlayerInfo->audioPacketQueue.serial)
        {
            qDebug()<<"audio MyPkt.serial "<<pMyPkt->serial<<"serial"<<pPlayerInfo->audioPacketQueue.serial;
            continue;
        }
        else
        {
            //qDebug()<<"AA MyPkt.serial "<<MyPkt.serial<<"serial"<<pVS->AudioPacketQueue.serial;
        }

        //得到一包有效pkt和frame
        ret = XFFmpeg::Get()->Decode(&pMyPkt->AVPkt, pFrame->frame);
        if (ret == 0)
        {
            qDebug()<<"Audio Dec error!";
            //av_packet_unref(&pMyPkt->AVPkt);
            av_free_packet(&pMyPkt->AVPkt);
            continue;
        }
        //qDebug()<<"Aduio ==> pts = "<<pFrame->frame->pts<<"count = "<<pVS->ADispQueue.Queue->count()<<"s ="<<pVS->ADispQueue.size;
        //av_packet_unref(&pMyPkt->AVPkt);
        av_free_packet(&pMyPkt->AVPkt);

        pFrame->serial = pMyPkt->serial;
        //adjustAVPts(XFFmpeg::Get()->audioStreamidx, pFrame);
        //qDebug()<<"A pFrame->serial = "<<pFrame->serial<<"pMyPkt->serial = "<<pMyPkt->serial;

        if (pPlayerInfo->ADispQueue.Queue->count() < pPlayerInfo->ADispQueue.size)
        {
            pPlayerInfo->ADispQueue.mutex.lock();
            pPlayerInfo->ADispQueue.Queue->append(pFrame);
            //qDebug()<<"audio ==> append";
            pFrame->DecState = DecOver;
            pFrame->DispState = DispWait;
            pPlayerInfo->ADispQueue.mutex.unlock();
        }

        //释放掉已经拿出来的节点内�?
        if (/*(pMyPkt->AVPkt.data != flush_pkt.data) && */(pMyPkt != NULL))
        {
            free((void *)pMyPkt);
            pMyPkt = NULL;
        }
    }

}

void AudioDecodeThread::initDecodeFrameQueue(PlayerInfo *pPI)
{
    for (int i = 0; i < FRAME_QUEUE_SIZE; i++)
    {
        if (!(pPI->audioFrameQueue.queue[i].frame = av_frame_alloc()))
        {
            qDebug()<<"alloc frame fail 2";
            return;
        }
        pPI->audioFrameQueue.queue[i].DecState = DecButt;
        pPI->audioFrameQueue.queue[i].DispState = DispButt;
    }
    pPI->audioFrameQueue.size = FRAME_QUEUE_SIZE;

    return;
}

void AudioDecodeThread::initPlayerInfo(PlayerInfo *pPI)
{
    if (pPI != NULL)
    {
        pPlayerInfo = pPI;
    }
}

void AudioDecodeThread::queueMessage(MessageCmd_t MsgCmd)
{
    pMessage->message_queue(MsgCmd);
}

AudioDecodeThread::~AudioDecodeThread()
{
    qDebug()<<"~AudioDisplayThread";
    this->terminate();//stop run thread
    for (int i = 0; i < pPlayerInfo->audioFrameQueue.size; i++)
    {
        if(pPlayerInfo->audioFrameQueue.queue[i].frame)
        {
            av_frame_unref(pPlayerInfo->audioFrameQueue.queue[i].frame);
            av_frame_free(&(pPlayerInfo->audioFrameQueue.queue[i].frame));
            pPlayerInfo->audioFrameQueue.queue[i].frame = NULL;
        }
    }

    if (pMessage)
    {
        delete pMessage;
    }
}

