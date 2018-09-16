#include "videodecodethread.h"
#include "ffmpeg.h"
#include <QDebug>

VideoDecodeThread::VideoDecodeThread()
{
    pPlayerInfo = NULL;

    pMessage = new message();
    if (!pMessage)
    {
        printf("AudioDecodeThread() error\n");
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

void VideoDecodeThread::run()
{
    int ret = -1;
    Frame *pFrame = NULL;
    AVPacket AvPkt = {0};
    myPacket *pMyPkt = NULL;
    AVCodecContext *codecCTX = NULL;
    qDebug()<<"VideoDecodeThread::run()";

    while(1)
    {
        if (!pPlayerInfo)
        {
            qDebug()<<" VideoDecodeThread pPlayerInfo empty !! :";
            msleep(10);
            continue;
        }

        if (pPlayerInfo->videoPacketQueue.Queue->isEmpty())
        {
            //qDebug()<<" Video Raw Queue empty !! :";
            msleep(10);
            continue;
        }

        if (!(pFrame = GetOneValidFrame(&pPlayerInfo->videoFrameQueue)))
        {
            //qDebug()<<" video GetOneValidFrame fail";
            msleep(10);
            continue;
        }

        if ((pMyPkt = GetOnePacket(&(pPlayerInfo->videoPacketQueue))) == NULL)
        {
            //qDebug()<<" video GetOnePacket fail";
            msleep(10);
            continue;
        }

        if (!XFFmpeg::Get()->ic)
        {
            msleep(10);
            continue;
        }
        codecCTX = XFFmpeg::Get()->ic->streams[XFFmpeg::Get()->videostreamidx]->codec;
        //qDebug()<<"video timebase num "<<codecCTX->time_base.num<<"den "<<codecCTX->time_base.den;
#if 0//todo
        if (pMyPkt->AVPkt.data == flush_pkt.data && codecCTX != NULL)
        {
            pPlayerInfo->VDispQueueDec.mutex.lock();
            pPlayerInfo->VDispQueueDec.Queue->clear();
            pPlayerInfo->VDispQueueDec.mutex.unlock();
            FlashFrameQueue(&pPlayerInfo->videoFrameQueue);
            avcodec_flush_buffers(codecCTX);
            pPlayerInfo->bVideoFlash = 1;
            qDebug()<<"video flash";
            continue;
        }
#endif
        if (pMyPkt->serial != pPlayerInfo->videoPacketQueue.serial)
        {
            continue;
        }

        //得到一包有效pkt和frame
        ret = XFFmpeg::Get()->Decode(&pMyPkt->AVPkt, pFrame->frame);
        if (ret == 0)
        {
            av_free_packet(&pMyPkt->AVPkt);
            //av_packet_unref(&pMyPkt->AVPkt);
            //qDebug()<<"Video Dec error!";
            continue;
        }
        //qDebug()<<"video ==> pts = "<<pFrame->frame->pts;
        //av_packet_unref(&pMyPkt->AVPkt);
        av_free_packet(&pMyPkt->AVPkt);

        pFrame->serial = pMyPkt->serial;
        //adjustAVPts(XFFmpeg::Get()->videostreamidx, pFrame);
        //qDebug()<<"V pFrame->serial = "<<pFrame->serial<<"pMyPkt->serial = "<<pMyPkt->serial;

        if (pPlayerInfo->VDispQueue.Queue->count() < pPlayerInfo->VDispQueue.size && pPlayerInfo->VDispQueue.Queue->count() >= 0)
        {
            pPlayerInfo->VDispQueue.mutex.lock();
            pPlayerInfo->VDispQueue.Queue->append(pFrame);
            pFrame->DecState = DecOver;
            pFrame->DispState = DispWait;
            pPlayerInfo->VDispQueue.mutex.unlock();
        }

        //释放掉已经拿出来的节点内�?
        if (/*(pMyPkt->AVPkt.data != flush_pkt.data) && */(pMyPkt != NULL))
        {
            free((void *)pMyPkt);
            pMyPkt = NULL;
        }
    }
}

void VideoDecodeThread::initPlayerInfo(PlayerInfo *pPI)
{
    if (pPI != NULL)
    {
        pPlayerInfo = pPI;
    }
}

void VideoDecodeThread::queueMessage(MessageCmd_t MsgCmd)
{
    pMessage->message_queue(MsgCmd);
}

void VideoDecodeThread::initDecodeFrameQueue(PlayerInfo *pPI)
{
    for (int i = 0; i < FRAME_QUEUE_SIZE; i++)
    {
        if (!(pPI->videoFrameQueue.queue[i].frame = av_frame_alloc()))
        {
            qDebug()<<"alloc frame fail 1";
            return;
        }
        pPI->videoFrameQueue.queue[i].DecState = DecButt;
        pPI->videoFrameQueue.queue[i].DispState = DispButt;
    }
    pPI->videoFrameQueue.size = FRAME_QUEUE_SIZE;
}

VideoDecodeThread::~VideoDecodeThread()
{
    qDebug()<<"~VideoDecodeThread()";

    this->terminate();//stop run thread
    for (int i = 0; i < pPlayerInfo->videoFrameQueue.size; i++)
    {
        if(pPlayerInfo->videoFrameQueue.queue[i].frame)
        {
            av_frame_unref(pPlayerInfo->videoFrameQueue.queue[i].frame);
            av_frame_free(&(pPlayerInfo->videoFrameQueue.queue[i].frame));
            pPlayerInfo->videoFrameQueue.queue[i].frame = NULL;
        }
    }

    if (pMessage)
    {
        delete pMessage;
    }
}
