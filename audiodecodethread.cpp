#include "audiodecodethread.h"
#include "ffmpeg.h"
#include "QDebug"

AudioDecodeThread::AudioDecodeThread()
{
    pPlayerInfo = NULL;
    bStop = 0;
    bStopDone = 0;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"AudioDecodeThread() error\n";
    }
}

Frame *AudioDecodeThread::GetOneValidFrame(FrameQueue *pQueue)
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

myPacket *AudioDecodeThread::GetOnePacket(PacketQueue *pPacketQueue)
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

int AudioDecodeThread::adjustAudioPts(Frame *pFrame)
{
    int deltaPts = 0;
    int needAdjust = 0;
    AVCodecContext *codecCTX = NULL;
    static int actually_delta_pts = 0;
    static int LastSerial = -1;
    static int SerialFirstPts = -1;
    static int LastFramePts = -1;
    static int cnt = 0;

    codecCTX = XFFmpeg::Get()->ic->streams[XFFmpeg::Get()->audioStreamidx]->codec;
    if (actually_delta_pts == 0)
    {
        actually_delta_pts = ((codecCTX->frame_size * 1000 ) / codecCTX->sample_rate);
        //qDebug()<<" Auido actually_delta_pts :"<<actually_delta_pts;
    }

    if (isFirstFrame == 1)
    {
        //将序列第一帧的pts记录下来
        SerialFirstPts = pFrame->frame->pts;
        LastFramePts = SerialFirstPts;
        isFirstFrame = 0;
        actually_delta_pts = 0;
        qDebug()<<"audio adjustAudioPts SerialFirstPts :"<<SerialFirstPts;
        return 0;
    }
    else
    {
        deltaPts = pFrame->frame->pts - LastFramePts;
        //当从实际pts算出的delta与根据fps算出来的理论delta要大很多或者小很多，说明pts是异常的
        if (abs(deltaPts - actually_delta_pts) >= 100)
        {
            qDebug()<<"deltaPts "<<deltaPts;
            qDebug()<<"actually_delta_pts "<<actually_delta_pts;
            qDebug()<<"AAAA abs : "<<abs(deltaPts - actually_delta_pts);
            needAdjust = 1;
        }
        LastFramePts = pFrame->frame->pts;
    }

    if (needAdjust == 1)
    {
        qDebug()<<"audio adjust before pts : "<<pFrame->frame->pts;
        int temp = (++cnt) * actually_delta_pts;
        pFrame->frame->pts =  SerialFirstPts + temp;
        qDebug()<<"audio adjust after pts : "<<pFrame->frame->pts;
        return 0;
    }

    //qDebug()<<"no needAdjust pts : "<<pFrame->frame->pts;
    return 0;

}

void AudioDecodeThread::stop()
{
    bStop = 1;
    isFirstFrame = 1;

    mutex.lock();
    while (bStopDone != 1)
    {
        if (!WaitCondStopDone.wait(&mutex, 2000))
        {
            qDebug()<<"AudioDecodeThread::stop  wait timeout";
            break;
        }
    }
    mutex.unlock();
}

void AudioDecodeThread::run()
{
    int ret = -1;
    Frame *pFrame = NULL;
    myPacket *pMyPkt = NULL;

    qDebug()<<"AudioDecodeThread::run() in";

    bStop = 0;
    while(!bStop)
    {
        if (pPlayerInfo == NULL)
        {
            msleep(10);
            qDebug()<<"AudioDecodeThread error";
            continue;
        }

        if (pPlayerInfo->audioPacketQueue.Queue->isEmpty())
        {
            qDebug()<<" audio Raw Queue empty !! ==> wait";
            pPlayerInfo->ADispQueue.mutex.lock();
            if (!pPlayerInfo->pWaitCondAudioDecodeThread->wait(&(pPlayerInfo->ADispQueue.mutex), 5000))
            {
                qDebug()<<"pWaitCondAudioDecodeThread wait timeout";
            }
            qDebug()<<" audio Raw Queue ==> wait ==>wakeup";
            pPlayerInfo->ADispQueue.mutex.unlock();
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
            continue;
        }

        if (!XFFmpeg::Get()->ic)
        {
            msleep(10);
            continue;
        }

        ret = XFFmpeg::Get()->Decode(&pMyPkt->AVPkt, pFrame->frame);
        if (ret == 0)
        {
            qDebug()<<"Audio Dec error!";
            av_free_packet(&pMyPkt->AVPkt);
            continue;
        }
        av_free_packet(&pMyPkt->AVPkt);

        //qDebug()<<"Aduio ==> pts = "<<pFrame->frame->pts;

        if (pPlayerInfo->ADispQueue.Queue->count() < pPlayerInfo->ADispQueue.size)
        {
            pPlayerInfo->ADispQueue.mutex.lock();
            pPlayerInfo->ADispQueue.Queue->append(pFrame);
            pPlayerInfo->pWaitCondAudioOutputThread->wakeAll();
            //qDebug()<<"audio ==> append";
            pFrame->DecState = DecOver;
            pFrame->DispState = DispWait;
            pPlayerInfo->ADispQueue.mutex.unlock();
        }

        //free malloc pkt
        if (pMyPkt != NULL)
        {
            free((void *)pMyPkt);
            pMyPkt = NULL;
        }
    }

    mutex.lock();
    bStopDone = 1;
    WaitCondStopDone.wakeAll();
    mutex.unlock();
    qDebug()<<"AudioDecodeThread stop!";

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

void AudioDecodeThread::deinitDecodeFrameQueue(PlayerInfo *pPI)
{
    if (pPI)
    {
        for (int i = 0; i < pPI->audioFrameQueue.size; i++)
        {
            if(pPI->audioFrameQueue.queue[i].frame)
            {
                av_frame_unref(pPI->audioFrameQueue.queue[i].frame);
                av_frame_free(&(pPI->audioFrameQueue.queue[i].frame));
                pPI->audioFrameQueue.queue[i].frame = NULL;
                pPI->audioFrameQueue.queue[i].DecState = DecButt;
                pPI->audioFrameQueue.queue[i].DispState = DispButt;
            }
        }
    }
}

void AudioDecodeThread::flushDecodeFrameQueue(PlayerInfo *pPI)
{
    for (int i = 0; i < pPI->audioFrameQueue.size; i++)
    {
        if(pPI->audioFrameQueue.queue[i].frame)
        {
            av_frame_unref(pPI->audioFrameQueue.queue[i].frame);
            pPI->audioFrameQueue.queue[i].DecState = DecButt;
            pPI->audioFrameQueue.queue[i].DispState = DispButt;
        }
    }
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
            pPlayerInfo->audioFrameQueue.queue[i].DecState = DecButt;
            pPlayerInfo->audioFrameQueue.queue[i].DispState = DispButt;
        }
    }

    if (pMessage)
    {
        delete pMessage;
    }
}

