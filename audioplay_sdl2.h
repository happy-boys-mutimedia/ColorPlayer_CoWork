#ifndef AUDIOPLAY_SDL2_H
#define AUDIOPLAY_SDL2_H
#include <QThread>
#include "colorplayer.h"
#include "messagequeue.h"
#include "masterclock.h"


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
    virtual ~SDL2AudioDisplayThread();
private:
    PlayerInfo *player;
    message *pMessage;
    MasterClock * pMasterClock;
    int bFirstFrame;
    int bStop;
};

#endif // AUDIOPLAY_SDL2_H
