#ifndef VIDEOOUTPUT_H
#define VIDEOOUTPUT_H
#include "colorplayer.h"
#include "messagequeue.h"
#include "QThread"

class VideoOutput:public QThread
{
public:
    static VideoOutput *Get()
    {
        static VideoOutput vt;
        return &vt;
    }

    void run();
    void initPlayerInfo(PlayerInfo *pPI);
    void initDisplayQueue(PlayerInfo *pPI);
    void queueMessage(MessageCmd_t MsgCmd);
    Frame *GetFrameFromDisplayQueue(PlayerInfo *pPI);
    void receiveFrametoDisplayQueue(Frame *pFrame);
    virtual ~VideoOutput();
private:
    VideoOutput();
    PlayerInfo *pPlayerInfo;
    message *pMessage;
    Frame *pPauseFrame;
};

#endif // VIDEOOUTPUT_H
