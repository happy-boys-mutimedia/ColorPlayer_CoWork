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
    bVideoFreeRun = 0;
    pMasterClock = NULL;
    bStop = 0;

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

int VideoOutput::NeedAVSync(MessageCmd_t MsgCmd, int bPaused)
{
    if ((MsgCmd.cmd == MESSAGE_CMD_PAUSE) || bPaused || bVideoFreeRun)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int VideoOutput::DecideKeepFrame(int need_av_sync, int64_t pts)
{
    int64_t late = 0;

    if (!need_av_sync)
        return 1;

    late = CalcSyncLate(pts);
    if (late < 0)
    {
        qDebug()<<"video pts is late need drop this frame.";
        return 0;
    }

    return 1;
}

int VideoOutput::CheckOutWaitOrDisplay(int64_t pts)
{
    int64_t late = 0;
    int beyond_tolerance_threshold = 15;//just test

    late = CalcSyncLate(pts);
    if (late >= 0)
    {
        // video beyond master clock
        if (late <= beyond_tolerance_threshold)
        {
            //qDebug()<<"CheckOutWaitOrDisplay go disp "<<late;
            return SYNC_GO_DISP;
        }
        else
        {
            //qDebug()<<"CheckOutWaitOrDisplay go wait beyond:"<<late;
            // video beyond master clock too much need delay.
            msleep(1);
            return SYNC_GO_WAIT;
        }
    }
    return SYNC_GO_DISP;
}

int64_t VideoOutput::CalcSyncLate(int64_t pts)
{
    int64_t current_master_time = 0;
    int64_t next_video_time = 0;

    current_master_time = pMasterClock->get_master_clock();
    next_video_time = pMasterClock->convert_to_system_time(pts);

    //qDebug()<<"current_master_time "<<current_master_time<<"next_video_time "<<next_video_time;
    // return next video pts is beyond master time or not.
    return (next_video_time - current_master_time);
}

void VideoOutput::stop()
{
    bStop = 1;
}

void VideoOutput::flush()
{
    if (pPauseFrame)
    {
        pPauseFrame->DecState = DecWait;
        pPauseFrame->DispState = DispOver;
        pPauseFrame = NULL;
    }

    bVideoFreeRun = 0;

    pPlayerInfo->Video2WidgetQueue.Queue->clear();
    pPlayerInfo->VDispQueue.Queue->clear();
}

void VideoOutput::run()
{
    int sync_status = SYNC_GO_NEXT;
    MessageCmd_t Msgcmd;
    Msgcmd.cmd = MESSAGE_CMD_NONE;
    Msgcmd.cmdType = MESSAGE_QUEUE_TYPES;

    threadState currentState = THREAD_NONE;
    int need_av_sync = 0;
    int bPaused = 0;

    qDebug()<<"VideoOutput::run() IN";
    bStop = 0;
    while (!bStop)
    {
        static Frame *pFrame = NULL;

        if (!pPlayerInfo)
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
                bPaused = 0;
            }
            //qDebug()<<"VideoOutput THREAD_PAUSE~~ ";
            msleep(10);
            continue;
        }

        if ((pMessage->message_dequeue(&Msgcmd) == SUCCESS) && (Msgcmd.cmd == MESSAGE_CMD_PAUSE))
        {
            qDebug()<<"VideoOutput get pause cmd~~ ";
            currentState = THREAD_PAUSE;
            bPaused = 1;
            msleep(10);
            continue;
        }

        // read queued video frame
        if (sync_status == SYNC_GO_NEXT)
        {
            pPlayerInfo->VDispQueue.mutex.lock();
            if (pPlayerInfo->VDispQueue.Queue->isEmpty())
            {
                qDebug()<<"VDispQueue empty";
                pPlayerInfo->VDispQueue.mutex.unlock();
                msleep(10);
                continue;
            }

            pFrame = pPlayerInfo->VDispQueue.Queue->takeFirst();
            pPlayerInfo->VDispQueue.mutex.unlock();
            if (pFrame == NULL || pFrame->frame == NULL)
            {
                qDebug()<<"pFrame == NULL";
                msleep(10);
                continue;
            }

            int KeepFrame = 1;
            need_av_sync = NeedAVSync(Msgcmd, bPaused);
            //need_av_sync = 0;//do not avsync
            KeepFrame = DecideKeepFrame(need_av_sync, pFrame->frame->pts);
            //qDebug()<<"VDispQueue takeFirst() KeepFrame "<<KeepFrame<<"need_av_sync "<<need_av_sync;
            if (!KeepFrame)
            {
                sync_status = SYNC_GO_DROP;
            }
            else if (!need_av_sync)
            {
                sync_status = SYNC_GO_DISP;
            }
            else
            {
                sync_status = SYNC_GO_WAIT;
            }
        }

        //check out need wait or display this frame
        if (sync_status == SYNC_GO_WAIT)
        {
            sync_status = CheckOutWaitOrDisplay(pFrame->frame->pts);
            //qDebug()<<"4444"<<"sync_status "<<sync_status;
        }

        if (sync_status == SYNC_GO_DROP)
        {
            sync_status = SYNC_GO_FREE;
        }

        if (sync_status == SYNC_GO_DISP)
        {
            pPlayerInfo->Video2WidgetQueue.mutex.lock();
            if (pPlayerInfo->Video2WidgetQueue.Queue->count() < pPlayerInfo->Video2WidgetQueue.size)
            {
                pPlayerInfo->Video2WidgetQueue.Queue->append(pFrame);
            }
            pPlayerInfo->Video2WidgetQueue.mutex.unlock();

            sync_status = SYNC_GO_NEXT;
        }

        if (sync_status == SYNC_GO_FREE)
        {
            //change state
            pFrame->DecState = DecWait;
            pFrame->DispState = DispOver;
            sync_status = SYNC_GO_NEXT;
        }
    }
    qDebug()<<"VideoOutput::run() stop!";
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

void VideoOutput::deinitDisplayQueue(PlayerInfo *pPI)
{
    if (pPI)
    {
        pPI->Video2WidgetQueue.Queue->clear();
        pPI->VDispQueue.Queue->clear();
    }
}

void VideoOutput::initMasterClock(MasterClock * pMC)
{
    if (pMC != NULL)
    {
        pMasterClock = pMC;
    }
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





