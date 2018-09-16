#ifndef READFILETHREAD_H
#define READFILETHREAD_H

#include <QThread>
#include "ffmpeg.h"
#include <QMutex>
#include <QList>
#include <QTimerEvent>
#include <QWidget>
#include "colorplayer.h"

#define SIZE 1000
//#define FRAME_QUEUE_SIZE 24
#define FULL(W,R,S) (((W) + 1) % (S) == (R))
#define EMPTY(R,W) ((R) == (W))
#define ABS(a) ((a) > 0) ? (a) : (-a)
#define FREE(R,W) (SIZE - ABSOLUTE(R,W))

#define NORMAL_SYNC 50      //正常同步区间100ms
#define InDelay_SYNC 2000    //可以通过delay时间来调整的阈值2000ms

class readfilethread:public QThread
{
public:
    static readfilethread *Get()
    {
        static readfilethread vt;
        return &vt;
    }
    void run();

    void initPlayerInfo(PlayerInfo *pPI);
    int initRawQueue(PlayerInfo *pPI);

    virtual ~readfilethread();
private:
    readfilethread();
    PlayerInfo *pPlayerInfo;
    //message *pMessage;
};

#endif // READFILETHREAD_H
