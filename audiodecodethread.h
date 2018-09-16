#ifndef AUDIODECODETHREAD_H
#define AUDIODECODETHREAD_H

#include <QThread>
#include "colorplayer.h"
#include "messagequeue.h"

class AudioDecodeThread:public QThread
{
public:
    static AudioDecodeThread *Get()
    {
        static AudioDecodeThread vt;
        return &vt;
    }

    void run();
    void initPlayerInfo(PlayerInfo *pPI);
    void initDecodeFrameQueue(PlayerInfo *pPI);
    void queueMessage(MessageCmd_t MsgCmd);
    virtual ~AudioDecodeThread();
private:
    AudioDecodeThread();
    PlayerInfo *pPlayerInfo;
    message *pMessage;
};

#endif // AUDIODECODETHREAD_H
