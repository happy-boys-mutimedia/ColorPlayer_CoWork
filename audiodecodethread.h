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
    void stop();
    void initPlayerInfo(PlayerInfo *pPI);
    void initDecodeFrameQueue(PlayerInfo *pPI);
    void deinitDecodeFrameQueue(PlayerInfo *pPI);
    void flushDecodeFrameQueue(PlayerInfo *pPI);
    void queueMessage(MessageCmd_t MsgCmd);
    virtual ~AudioDecodeThread();
private:
    AudioDecodeThread();
    int adjustAudioPts(Frame *pFrame);
    Frame *GetOneValidFrame(FrameQueue *pQueue);
    myPacket *GetOnePacket(PacketQueue *pPacketQueue);
    int isFirstFrame = 1;
    PlayerInfo *pPlayerInfo;
    message *pMessage;
    int bStop;
};

#endif // AUDIODECODETHREAD_H
