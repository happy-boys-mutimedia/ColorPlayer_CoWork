#include "videooutput.h"
#include <Qlist>
#include <QDebug>

QList<Frame *>videoDispDec;
QList<Frame *>videoDispSync;

enum {
    SYNC_GO_NEXT,
    SYNC_GO_WAIT,
    SYNC_GO_DISP,
    SYNC_GO_DROP,
    SYNC_GO_CONVERT,
    SYNC_GO_FREE
};

VideoOutput::VideoOutput()
{
    pPlayerInfo = NULL;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"AudioDecodeThread() error\n";
    }
}

VideoOutput::~VideoOutput()
{
    qDebug()<<"~VideoOutput()";
    this->terminate();//stop run thread

    if (pMessage)
    {
        delete pMessage;
    }
}

void VideoOutput::run()
{
    int sync_status = SYNC_GO_NEXT;
    MessageCmd_t Msgcmd;
    Msgcmd.cmd = MESSAGE_CMD_NONE;
    Msgcmd.cmdType = MESSAGE_QUEUE_TYPES;

    threadState currentState = THREAD_NONE;

    while (1)
    {
#if 0
        pMessage->message_dequeue(&Msgcmd);

        if (Msgcmd.cmd == MESSAGE_CMD_PAUSE)
        {
            msleep(10);
            continue;
        }

        if (Msgcmd.cmd == MESSAGE_CMD_STOP)
            break;

        if (Msgcmd.cmd == MESSAGE_CMD_FLUSH)
        {
            //todo
        }

        // read queued video frame
        if (sync_status == SYNC_GO_NEXT)
        {
            //todo av sync
        }
#endif

        static Frame *pFrame = NULL;
        //qDebug()<<"video timeSetEvent: "<<getTimeInMs();
        if (!pPlayerInfo || !pPlayerInfo->isInitAll)
        {
            //qDebug()<<"not init ! ";
            msleep(10);
            continue;
        }

        if (currentState == THREAD_PAUSE)
        {
            if ((pMessage->message_dequeue(&Msgcmd) == SUCCESS) && (Msgcmd.cmd == MESSAGE_CMD_RESUME))
            {
                qDebug()<<"VideoOutput get resume cmd~~ ";
                currentState = THREAD_START;
            }
            //qDebug()<<"VideoOutput THREAD_PAUSE~~ ";
            msleep(10);
            continue;
        }

        if ((pMessage->message_dequeue(&Msgcmd) == SUCCESS) && (Msgcmd.cmd == MESSAGE_CMD_PAUSE))
        {
            qDebug()<<"VideoOutput get pause cmd~~ ";
            currentState = THREAD_PAUSE;
            msleep(10);
            continue;
        }

        pPlayerInfo->VDispQueue.mutex.lock();
        if (pPlayerInfo->VDispQueue.Queue->isEmpty())
        {
            //qDebug()<<"VDispQueue empty";
            pPlayerInfo->VDispQueue.mutex.unlock();
            msleep(10);
            continue;
        }
        //qDebug()<<"get one frame form VDispQueue";
        pFrame = pPlayerInfo->VDispQueue.Queue->takeFirst();

        pPlayerInfo->VDispQueue.mutex.unlock();

        if (pFrame == NULL || pFrame->frame == NULL)
        {
            qDebug()<<"pFrame == NULL";
            msleep(10);
            continue;
        }
    #if 0// video free run
        audioPts = getClockPts(&(pPlayerInfo->audclk));//以音频pts为准进行同步
        ret = isSyncVideo2Audio(pFrame->frame->pts, audioPts);
        if (ret == needDiscard)
        {
            //需要将此帧丢弃
            isLastFrameSync = 1;
            pFrame->DecState = DecWait;
            pFrame->DispState = DispOver;
            qDebug()<<"video needDiscard frame";
            return;
        }
        else if (ret == needDelay)
        {
            isLastFrameSync = 0;
            return;
        }
    #endif

        pPlayerInfo->Video2WidgetQueue.mutex.lock();
        if (pPlayerInfo->Video2WidgetQueue.Queue->count() < pPlayerInfo->Video2WidgetQueue.size)
        {
            pPlayerInfo->Video2WidgetQueue.Queue->append(pFrame);
        }
        pPlayerInfo->Video2WidgetQueue.mutex.unlock();

        //显示完成更新帧存状态，解码线程可以拿帧继续解码
        //pFrame->DecState = DecWait;
        //pFrame->DispState = DispOver;

    }
    //todo free/close
}

void VideoOutput::initPlayerInfo(PlayerInfo *pPI)
{
    if (pPI != NULL)
    {
        pPlayerInfo = pPI;
    }

    pPauseFrame = NULL;
}

void VideoOutput::initDisplayQueue(PlayerInfo *pPI)
{
    pPI->VDispQueue.Queue = &videoDispDec;
    pPI->VDispQueue.size = FRAME_QUEUE_SIZE + 1;
    pPI->Video2WidgetQueue.Queue = &videoDispSync;
    pPI->Video2WidgetQueue.size = FRAME_QUEUE_SIZE + 1;
}

void VideoOutput::queueMessage(MessageCmd_t MsgCmd)
{
    pMessage->message_queue(MsgCmd);
}

Frame* VideoOutput::GetFrameFromDisplayQueue(PlayerInfo *pPI)
{
    Frame *pFrame = NULL;

    //for pause state to rescale window,give pause frame to display
    if ((pPlayerInfo->playerState == PLAYER_STATE_PAUSE) && (pPauseFrame != NULL))
    {
        return pPauseFrame;
    }

    if (pPI->Video2WidgetQueue.Queue->isEmpty())
    {
        return NULL;
    }

    pPI->Video2WidgetQueue.mutex.lock();
    if (!pPI->Video2WidgetQueue.Queue->isEmpty())
    {
        pFrame = pPI->Video2WidgetQueue.Queue->takeFirst();
    }
    pPI->Video2WidgetQueue.mutex.unlock();

    //for pause state to rescale window,give pause frame to display
    if (pPlayerInfo->playerState == PLAYER_STATE_PAUSE)
    {
        pPauseFrame = pFrame;
    }
    else if (pPauseFrame != NULL)
    {
        pPauseFrame = NULL;
    }

    return pFrame;
}

void VideoOutput::receiveFrametoDisplayQueue(Frame *pFrame)
{
    //do not change frame state for decoder can't use it.
    if ((pPlayerInfo->playerState == PLAYER_STATE_PAUSE) && (pPauseFrame != NULL))
    {
        return;
    }
    else if (pPauseFrame != NULL)
    {
        //reset pPauseFrame state for which decoder can use it again
        pPauseFrame->DecState = DecWait;
        pPauseFrame->DispState = DispOver;
    }
    pFrame->DecState = DecWait;
    pFrame->DispState = DispOver;
}





