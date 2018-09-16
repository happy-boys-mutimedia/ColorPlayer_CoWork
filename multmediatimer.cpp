#include "multmediatimer.h"
#include "ffmpeg.h"
#include "colorplayer.h"
#include "audioplay.h"
#include "videowidget.h"
#include <QTime>
#include <QDebug>

extern PlayerInfo *pVS;

typedef enum syncFlage
{
    needDiscard = 0,
    isSync,
    needDelay,
    nutt,
}syncFlage;


static double r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

static int getTimeInMs()
{
    QTime current_time = QTime::currentTime();
    int hour = current_time.hour();//当前的小时
    int minute = current_time.minute();//当前的分
    int second = current_time.second();//当前的秒
    int msec = current_time.msec();//当前的毫秒

    return (hour * 60 * 60 * 1000 + minute * 60 * 1000 + second * 1000 + msec);
}

static void updateClock(Clock *pClock, int pts, int64_t masterTime)
{
    if (pts >= 0)
    {
        pClock->pts = pts;
    }

    pClock->last_updated = masterTime;//ms级别
    pClock->pts_drift = pClock->pts - pClock->last_updated;//ms级别
    //qDebug()<<"updateClock pts"<<pts;
    return;
}

static int checkIsSync(Clock *pClock, int64_t currentPts, int64_t cur_masterTime)
{
    int64_t syncTimeMs = 0;
    int64_t needSyncPts = currentPts;
    double duration = 0;
    double dfFreq;

    //duration = (cur_masterTime - pClock->last_updated) / dfFreq;//计算出上次更新的时候到现在的时候过了多久,秒为单位
    syncTimeMs = cur_masterTime + pClock->pts_drift;//换算为ms为单位
    //qDebug()<<"syncTimeMs = "<<syncTimeMs<<"cur_masterTime = "<<cur_masterTime<<"currentPts "<<currentPts;

    if ( syncTimeMs >= needSyncPts)
    {
        if (syncTimeMs - needSyncPts)
        {
            //qDebug()<<"syncTimeMs - needSyncPts = "<<syncTimeMs - needSyncPts;
        }
        return 0;//说明已经同步上
    }
    else
    {
        return -1;
    }
}

static void CALLBACK AUDIOCallBackFun(UINT wTimerID, UINT msg,DWORD dwUser, DWORD dwl,DWORD dw2)
{
    //qDebug()<<"timeSetEvent: "<<getTimeInMs();

    char out[15000] = { 0 };
    int i = 0;
    static Frame *pFrame;
    AVFormatContext *ictx = NULL;
    AVCodecContext *codecCTX = NULL;
    LONGLONG masterTime = 0;
    int ret = -1;
    static int isLastFrameSync = 1;
    static int isFirstFrame = 1;
    static int LastSerial = -1;
    int delayUs = 0;

    if (!pVS)
    {
        return;
    }

    if (!pVS->isInitAll)
    {
        return;
    }

    if ((!XFFmpeg::Get()->isPlay) && pVS->canReadFile)
    {
        //qDebug()<<"canReadFile = "<<pVS->canReadFile;
        return;
    }

    pVS->ADispQueue.mutex.lock();
    if (pVS->ADispQueue.Queue->isEmpty())
    {
        //qDebug()<<"audio disque empty!!";
        pVS->ADispQueue.mutex.unlock();
        return;
    }

    //如果上一帧还没有同步出来显示，不拿新的帧
    if (isLastFrameSync == 1 || isFirstFrame == 1)
    {
        pFrame = pVS->ADispQueue.Queue->takeFirst();
        //qDebug()<<" get pts ===>"<<pFrame->frame->pts;
    }
    pVS->ADispQueue.mutex.unlock();

    if (pFrame == NULL)
    {
        //qDebug()<<"AAAAAAAAAA";
        return;
    }

    if (pFrame->frame == NULL)
    {
        //qDebug()<<"BBBBBBBB";
        return;
    }

    if (pFrame->serial != LastSerial)
    {
        qDebug()<<"a disp pFrame->serial "<<pFrame->serial<<"LastSerial "<<LastSerial;
        //说明流已经更新
        LastSerial = pFrame->serial;
        isFirstFrame = 1;
        return;
    }

    if (isFirstFrame == 0)//如果是系列第一帧不进行同步判断
    {
        masterTime = getTimeInMs();//系统时间
        //qDebug()<<"masterTime "<<masterTime<<"pts "<<pFrame->frame->pts;
        ret = checkIsSync(&(pVS->audclk), pFrame->frame->pts, masterTime);
        if (ret != 0)//未达到pts时间cuo
        {
            isLastFrameSync = 0;//说明这一帧还没有到显示的时候，需要delay,先保存起来
            //qDebug()<<"need delay====>";
            return;
        }
    }

    //可以显示了
    isLastFrameSync = 1;//这一帧已经显示
    isFirstFrame = 0;//第一帧已经显示，标志为零
    LastSerial = pFrame->serial;

    int64_t MasterTime = getTimeInMs();
    updateClock(&(pVS->audclk), pFrame->frame->pts, MasterTime);

    //qDebug()<<"update time"<<MasterTime<<"pts "<<pFrame->frame->pts;

    XFFmpeg::Get()->PutFrameToConvert(XFFmpeg::Get()->audioStreamidx, pFrame->frame);
    int len = XFFmpeg::Get()->ToPCM(out);
    if (len == 0)
    {
        qDebug()<<"ToPCM error !";
        pFrame->DecState = DecWait;
        pFrame->DispState = DispOver;
        av_frame_unref(pFrame->frame);
        return;
    }
    AudioPlay::Get()->Write(out, len);

    ictx = XFFmpeg::Get()->ic;
    //更新音频pts用于界面显示时间
    XFFmpeg::Get()->pts = pFrame->frame->pts; //* r2d(ictx->streams[XFFmpeg::Get()->audioStreamidx]->time_base) * 1000;
    //qDebug()<<"finally pts "<<XFFmpeg::Get()->pts<<" -- "<<r2d(ictx->streams[XFFmpeg::Get()->audioStreamidx]->time_base)<<"pts "<<pFrame->frame->pts;

    //计算需要delay的时间
    codecCTX = XFFmpeg::Get()->ic->streams[XFFmpeg::Get()->audioStreamidx]->codec;
    if (delayUs == 0)
    {
        delayUs = ((codecCTX->frame_size * 1000 ) / codecCTX->sample_rate) * 1000;
        //qDebug()<<"frame_size"<<codecCTX->frame_size<<"sample_rate"<<codecCTX->sample_rate<<"delayus "<<delayUs;
    }

    //显示完成更新帧存状态，解码线程可以拿帧继续解码
    pFrame->DecState = DecWait;
    pFrame->DispState = DispOver;

    return;
}

static int getClockPts(Clock *pClock)
{
    return pClock->pts;
}

static syncFlage isSyncVideo2Audio(int64_t videoPts, int64_t audioPts)
{
    syncFlage ret = nutt;

    if (videoPts < audioPts)
    {
        ret = needDiscard;//表示需要丢弃掉此帧
    }
    else if (videoPts >= audioPts && (videoPts - audioPts) < NORMAL_SYNC)
    {
        ret = isSync;//表示同步上了
    }
    else if (videoPts >= audioPts && (videoPts - audioPts) >= NORMAL_SYNC)
    {
        ret = needDelay;//表示需要delay
    }

    //qDebug()<<"ret = "<<needDelay<<"videoPts "<<videoPts<<"audioPts "<<audioPts;
    return ret;
}

static void CALLBACK VIDEOCallBackFun(UINT wTimerID, UINT msg,DWORD dwUser, DWORD dwl,DWORD dw2)
{
    static Frame *pFrame = NULL;
    static int isLastFrameSync = 1;
    int64_t audioPts = 0;
    syncFlage ret = nutt;

    //qDebug()<<"video timeSetEvent: "<<getTimeInMs();
    if (!pVS || !pVS->isInitAll)
    {
        qDebug()<<"not init ! ";
        return;
    }

    if (!XFFmpeg::Get()->isPlay && pVS->canReadFile)
    {
        return;
    }

    pVS->VDispQueue.mutex.lock();
    if (pVS->VDispQueue.Queue->isEmpty())
    {
        //qDebug()<<"VDispQueue empty";
        pVS->VDispQueue.mutex.unlock();
        return;
    }

    if (isLastFrameSync == 1)
    {
        pFrame = pVS->VDispQueue.Queue->takeFirst();
    }
    pVS->VDispQueue.mutex.unlock();

    if (pFrame == NULL || pFrame->frame == NULL)
    {
        //qDebug()<<"pFrame == NULL";
        return;
    }
#if 0// video free run
    audioPts = getClockPts(&(pVS->audclk));//以音频pts为准进行同步
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
    isLastFrameSync = 1;

    pVS->Video2WidgetQueue.mutex.lock();
    if (pVS->Video2WidgetQueue.Queue->count() < pVS->Video2WidgetQueue.size)
    {
        pVS->Video2WidgetQueue.Queue->append(pFrame);
    }
    pVS->Video2WidgetQueue.mutex.unlock();

    //显示完成更新帧存状态，解码线程可以拿帧继续解码
    pFrame->DecState = DecWait;
    //pFrame->DispState = DispOver;
}

void MultMediaTimer::TimerRun()
{
    //m_AudioTimerID = timeSetEvent(1, 1, AUDIOCallBackFun, (DWORD)this, TIME_PERIODIC);
    //m_VideoTimerID = timeSetEvent(1, 1, VIDEOCallBackFun, (DWORD)this, TIME_PERIODIC);
}

MultMediaTimer::MultMediaTimer()
{

}

MultMediaTimer::~MultMediaTimer()
{
    // 清除定时器
    //qDebug()<<"22222222222";
    //timeKillEvent(m_mmTimerID);
}
