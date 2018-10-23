#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H
#include "colorplayer.h"
#include "messagequeue.h"
#include "QThread"
#include <QMutex>
#include <QWaitCondition>

class DemuxThread:public QThread
{
public:
    static DemuxThread *Get()
    {
        static DemuxThread vt;
        return &vt;
    }
    void run();
    void stop();
    void initPlayerInfo(PlayerInfo *pPI);
    int initRawQueue(PlayerInfo *pPI);
    void deinitRawQueue(PlayerInfo *pPI);
    void queueMessage(MessageCmd_t MsgCmd);

    virtual ~DemuxThread();
private:
    DemuxThread();
    PlayerInfo *pPlayerInfo;
    message *pMessage;
    int bStop;
    int bStopDone;
    int bFirstVideoPkt;
    QMutex mutex;
    QWaitCondition WaitCondStopDone;
};

#endif // DEMUXTHREAD_H
