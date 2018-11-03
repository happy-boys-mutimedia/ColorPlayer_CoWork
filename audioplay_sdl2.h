#ifndef AUDIOPLAY_SDL2_H
#define AUDIOPLAY_SDL2_H
#include <QThread>
#include "colorplayer.h"
#include "messagequeue.h"
#include "masterclock.h"
#include "stdint.h"
#include <QMutex>
#include <QWaitCondition>


class SDL2AudioDisplayThread:public QThread
{
public:
    static SDL2AudioDisplayThread *Get(void)
    {
        static SDL2AudioDisplayThread vt;
        return &vt;
    }
    SDL2AudioDisplayThread();
    void init();
    void deinit();
    void run();
    void stop();
    void flush();
    void initPlayerInfo(PlayerInfo *pPI);
    void initDisplayQueue(PlayerInfo *pPI);
    void deinitDisplayQueue(PlayerInfo *pPI);
    void initMasterClock(MasterClock * pMC);
    void queueMessage(MessageCmd_t MsgCmd);
    void setCallback(pFuncCallback callback);
    virtual ~SDL2AudioDisplayThread();
    PlayerInfo *player;
    MasterClock * pMasterClock;
    int bFirstFrame;
    pFuncCallback _funcCallback;
private:
    PCMBuffer_t PCMBuffers[FRAME_QUEUE_SIZE];
    message *pMessage;
    int bStop;
    PCMBuffer_t *GetOneValidPCMBuffer();
    QMutex mutex;
    QWaitCondition WaitCondStopDone;
};

#endif // AUDIOPLAY_SDL2_H
