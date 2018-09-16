#include "readfilethread.h"
#include "ffmpeg.h"
#include <qdebug>
#include <QWaitCondition>
#include <QMutex>
#include <QTime>
#include <QApplication>
#include <QWaitCondition>
#include "audiodecodethread.h"
#include "audioplay_sdl2.h"
#include "videodecodethread.h"
#include "videooutput.h"

PlayerInfo *pVS = NULL;
QList<myPacket *>videoPacket;
QList<myPacket *>audioPacket;

static myPacket myFlush_pkt;
static AVPacket flush_pkt;
#define MAX_SIZE (2 * 1024 * 1024)


static double r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

int readfilethread::initRawQueue(PlayerInfo *pPI)
{
    pPI->videoPacketQueue.Queue = &videoPacket;
    pPI->audioPacketQueue.Queue = &audioPacket;

    //for flush
    av_init_packet(&flush_pkt);
    flush_pkt.data = (uint8_t *)&flush_pkt;
    myFlush_pkt.AVPkt = flush_pkt;

    return SUCCESS;
}

void readfilethread::initPlayerInfo(PlayerInfo *pPI)
{
    if (pPI != NULL)
    {
        pPlayerInfo = pPI;
    }
}

static void flashPacketQueue(PacketQueue *pPacketQueue)
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
                //qDebug()<<"tmp = "<<tmp++;
                free(pMyPkt);
            }
        }
    }

    if (pPacketQueue->Queue->isEmpty())
    {
        qDebug()<<"list is empty ";
    }

    return;
}

void readfilethread::run()
{
    int ret = -1;
    int canReadFile = 1;
    myPacket *tempMyPkt = NULL;

    qDebug()<<"readfilethread::run() in";
    while(1)
    {
        if ((!XFFmpeg::Get()->isPlay)|| !pPlayerInfo)
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
            //tempMyPkt->AVPkt = pkt;
            memset(tempMyPkt, 0, sizeof(myPacket));
            memcpy(&tempMyPkt->AVPkt, &pkt, sizeof(AVPacket));
            tempMyPkt->serial = pPlayerInfo->audioPacketQueue.serial;
            pPlayerInfo->audioPacketQueue.Queue->append(tempMyPkt);
            //qDebug()<<"A  ==> pkt.pts = "<<tempMyPkt->AVPkt.pts;
            //av_free_packet(&pkt);
            pPlayerInfo->audioPacketQueue.Mutex.unlock();
            continue;
        }

        if (pkt.stream_index == XFFmpeg::Get()->videostreamidx)//如果是视频包
        {
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
}

readfilethread::readfilethread()
{
    //qDebug()<<"readfilethread IN";
}

readfilethread::~readfilethread()
{
    qDebug()<<"~readfilethread release mem";
    flashPacketQueue(&pPlayerInfo->videoPacketQueue);
    flashPacketQueue(&pPlayerInfo->audioPacketQueue);
}
