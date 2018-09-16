#ifndef AUDIOPLAY_SDL2_H
#define AUDIOPLAY_SDL2_H
#include <QThread>
#include "colorplayer.h"
#include "messagequeue.h"

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
    void run();
    void initPlayerInfo(PlayerInfo *pPI);
    void initDisplayQueue(PlayerInfo *pPI);
    void queueMessage(MessageCmd_t MsgCmd);
    virtual ~SDL2AudioDisplayThread();
private:
    PlayerInfo *player;
    message *pMessage;
};

int sdl2_init(void);
int sdl2_display(void);

#endif // AUDIOPLAY_SDL2_H
