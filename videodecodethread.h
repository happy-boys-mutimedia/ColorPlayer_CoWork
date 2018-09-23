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
    void stop();
    void initPlayerInfo(PlayerInfo *pPI);
    void initDecodeFrameQueue(PlayerInfo *pPI);
    void deinitDecodeFrameQueue(PlayerInfo *pPI);
    void flushDecodeFrameQueue(PlayerInfo *pPI);
    void queueMessage(MessageCmd_t MsgCmd);
    virtual ~VideoDecodeThread();
private:
    VideoDecodeThread();
    Frame *GetOneValidFrame(FrameQueue *pQueue);
    myPacket *GetOnePacket(PacketQueue *pPacketQueue);
    int adjustVideoPts(Frame *pFrame);
    int isFirstFrame = 1;
    PlayerInfo *pPlayerInfo;
    message *pMessage;
    int bStop;
};

#endif // VIDEODECODETHREAD_H
