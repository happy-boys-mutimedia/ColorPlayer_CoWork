#ifndef VIDEODECODETHREAD_H
#define VIDEODECODETHREAD_H

#include <QThread>
#include "colorplayer.h"
#include "messagequeue.h"

class VideoDecodeThread:public QThread
{
public:
    static VideoDecodeThread *Get()
    {
        static VideoDecodeThread vt;
        return &vt;
    }

    void run();
    void initPlayerInfo(PlayerInfo *pPI);
    void initDecodeFrameQueue(PlayerInfo *pPI);
    void queueMessage(MessageCmd_t MsgCmd);
    virtual ~VideoDecodeThread();
private:
    VideoDecodeThread();
    PlayerInfo *pPlayerInfo;
    message *pMessage;
};

#endif // VIDEODECODETHREAD_H
