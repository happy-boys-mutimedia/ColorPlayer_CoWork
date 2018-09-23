#include "demuxthread.h"
#include <QList>
#include "ffmpeg.h"
#include <QDebug>

QList<myPacket *>videoPacket;
QList<myPacket *>audioPacket;
static myPacket myFlush_pkt;
static AVPacket flush_pkt;
#define MAX_SIZE (1 * 512 *1024)//1M

static double r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

static void flushPacketQueue(PacketQueue *pPacketQueue)
{
    int i;
    int tmp = 0;
    myPacket *pMyPkt = NULL;
    int listSize = pPacketQueue->Queue->count();

    qDebug()<<"listSize = "<<listSize;
    for (i = 0; i < listSize; i++)
    {
        if (!pPacketQueue->Queue->isEmpty())
        {
            pMyPkt = pPacketQueue->Queue->takeFirst();
            if (pMyPkt && (pMyPkt->AVPkt.data != flush_pkt.data))
            {
                av_free_packet(&pMyPkt->AVPkt);
                free(pMyPkt);
            }
        }
    }
    pPacketQueue->Queue->clear();

    return;
}

DemuxThread::DemuxThread()
{
    pPlayerInfo = NULL;
    bStop = 0;
    bFirstVideoPkt = 1;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"DemuxThread() error\n";
    }
}
DemuxThread::~DemuxThread()
{
    qDebug()<<"~DemuxThread()";
    this->terminate();//stop run thread

    flushPacketQueue(&pPlayerInfo->videoPacketQueue);
    flushPacketQueue(&pPlayerInfo->audioPacketQueue);

    if (pMessage)
    {
        delete pMessage;
    }
}

void DemuxThread::stop()
{
    bStop = 1;
}

void DemuxThread::run()
{
    int ret = -1;
    myPacket *tempMyPkt = NULL;

    bStop = 0;
    bFirstVideoPkt = 1;
    qDebug()<<"DemuxThread::run() in";
    while(!bStop)
    {
        if (!pPlayerInfo)
        {
            msleep(10);
            continue;
        }

        if ((pPlayerInfo->videoPacketQueue.Queue->count() + pPlayerInfo->audioPacketQueue.Queue->count()) * sizeof(AVPacket) > MAX_SIZE)
        {
            msleep(10);
            //qDebug()<<"full ===="<<"videoPacket :"<<pPlayerInfo->videoPacketQueue.Queue->count();
            //qDebug()<<"full ===="<<"audioPacket :"<<pPlayerInfo->audioPacketQueue.Queue->count();
            continue;
        }

        AVPacket pkt = XFFmpeg::Get()->Read();//读取一包
        if (pkt.size <= 0)
        {
            msleep(10);
            continue;
        }

        if (pkt.stream_index == XFFmpeg::Get()->audioStreamidx)//如果是音频包
        {
            pPlayerInfo->audioPacketQueue.Mutex.lock();
            tempMyPkt = (myPacket *)malloc(sizeof(myPacket));
            if (tempMyPkt == NULL)
            {
                qDebug()<<"no more memeory";
                continue;
            }

            memset(tempMyPkt, 0, sizeof(myPacket));
            memcpy(&tempMyPkt->AVPkt, &pkt, sizeof(AVPacket));
            tempMyPkt->serial = pPlayerInfo->audioPacketQueue.serial;
            pPlayerInfo->audioPacketQueue.Queue->append(tempMyPkt);
            //qDebug()<<"A  ==> pkt.pts = "<<tempMyPkt->AVPkt.pts;
            pPlayerInfo->audioPacketQueue.Mutex.unlock();
            continue;
        }

        if (pkt.stream_index == XFFmpeg::Get()->videostreamidx)//如果是视频包
        {
            //qDebug()<<"video bFirstVideoPkt"<<bFirstVideoPkt<<"keyframe :"<<(pkt.flags & AV_PKT_FLAG_KEY);
            if (bFirstVideoPkt && !(pkt.flags & AV_PKT_FLAG_KEY))
            {
                qDebug()<<"video first frame but not key frame continue==>";
                continue;
            }

            bFirstVideoPkt = 0;

            pPlayerInfo->videoPacketQueue.Mutex.lock();
            tempMyPkt = (myPacket *)malloc(sizeof(myPacket));
            if (tempMyPkt == NULL)
            {
             qDebug()<<"no more memeory";
             continue;
            }
            memset(tempMyPkt, 0, sizeof(myPacket));
            memcpy(&tempMyPkt->AVPkt, &pkt, sizeof(AVPacket));
            tempMyPkt->serial = pPlayerInfo->videoPacketQueue.serial;
            pPlayerInfo->videoPacketQueue.Queue->append(tempMyPkt);
            //qDebug()<<"V  ==> pkt.pts =  "<<tempMyPkt->AVPkt.pts;
            //av_free_packet(&pkt);
            pPlayerInfo->videoPacketQueue.Mutex.unlock();
        }
    }
    qDebug()<<"DemuxThread::run stop!";
}

int DemuxThread::initRawQueue(PlayerInfo *pPI)
{
    pPI->videoPacketQueue.Queue = &videoPacket;
    pPI->audioPacketQueue.Queue = &audioPacket;

    //for flush
    av_init_packet(&flush_pkt);
    flush_pkt.data = (uint8_t *)&flush_pkt;
    myFlush_pkt.AVPkt = flush_pkt;

    return SUCCESS;
}

void DemuxThread::deinitRawQueue(PlayerInfo *pPI)
{
    bFirstVideoPkt = 1;
    flushPacketQueue(&pPI->audioPacketQueue);
    flushPacketQueue(&pPI->videoPacketQueue);
}

void DemuxThread::queueMessage(MessageCmd_t MsgCmd)
{
    pMessage->message_queue(MsgCmd);
}

void DemuxThread::initPlayerInfo(PlayerInfo *pPI)
{
    if (pPI != NULL)
    {
        pPlayerInfo = pPI;
    }
}
