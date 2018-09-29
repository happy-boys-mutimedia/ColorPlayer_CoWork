#ifndef COLORPLAYER_H
#define COLORPLAYER_H
#include "ffmpeg.h"
#include "common.h"
#include "masterclock.h"
#include "stdint.h"

//#define FAILED    1
//#define SUCCESS   0
#define FRAME_QUEUE_SIZE 24

typedef enum eDisplayState
{
    DispWait = 0,
    DispOver = 1,
    DispButt = 2,
}displayState;

typedef enum eDecState
{
    DecWait = 0,
    DecOver = 1,
    DecButt = 2,
}decstate;

typedef struct myPacket
{
    AVPacket AVPkt;
    int serial;
}myPacket;

typedef struct PacketQueue {
    QList<myPacket *> *Queue;
    int nb_packets;
    int size;
    int64_t duration;
    int abort_request;
    int serial;
    QMutex Mutex;
} PacketQueue;

typedef struct Frame {
    AVFrame *frame;
    displayState DispState;
    decstate DecState;
    AVSubtitle sub;
    AVSubtitleRect **subrects;  /* rescaled subtitle rectangles in yuva */
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;          /* byte position of the frame in the input file */
    int allocated;
    int reallocate;
    int width;
    int height;
    AVRational sar;
} Frame;

typedef struct FrameQueue {
    Frame queue[FRAME_QUEUE_SIZE];
    int rindex;
    int windex;
    int size;
    int max_size;
    int keep_last;
    int rindex_shown;
    QMutex Mutex;
    PacketQueue *pktq;
} FrameQueue;

typedef struct DispFrameQueue
{
    QList<Frame *> *Queue;
    int rindex;
    int windex;
    int notUseNum;
    int size;
    QMutex mutex;
}DispFrameQueue;

typedef enum
{
    DISP_NONE,
    DISP_WAIT,
    DISP_DONE
} PCMBufferState_e;

typedef struct stPCMBuffer
{
    char *bufferAddr;
    uint bufferSize;
    int64_t pts;
    PCMBufferState_e state;
} PCMBuffer_t;

typedef struct DispPCMQueue
{
    QList<PCMBuffer_t *> *Queue;
    int rindex;
    int windex;
    int notUseNum;
    int size;
    QMutex mutex;
} DispPCMQueue;

typedef struct Clock {
    int64_t pts;           /* clock base */
    int64_t pts_drift;     /* clock base minus time at which we updated the clock */
    int64_t last_updated;
    double speed;
    int serial;           /* clock is based on a packet with this serial */
    int paused;
    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;

enum PlayerState
{
    PLAYER_STATE_NONE,
    PLAYER_STATE_PAUSE,
    PLAYER_STATE_RESUME,
    PLAYER_STATE_SEEK,
    PLAYER_STATE_START,
    PLAYER_STATE_STOP,
    PLAYER_STATE_FLUSH,
    PLAYER_STATE_FORCE_EOF,
    PLAYER_STATE_REINIT,
};

typedef struct PlayerInfo {
    PacketQueue videoPacketQueue;
    PacketQueue audioPacketQueue;
    DispFrameQueue VDispQueue;
    DispFrameQueue Video2WidgetQueue;
    DispFrameQueue ADispQueue;
    DispPCMQueue ADispPCMQueue;
    FrameQueue videoFrameQueue;
    FrameQueue audioFrameQueue;
    PlayerState playerState;
}PlayerInfo;

class ColorPlayer
{
public:
    static ColorPlayer *Get()
    {
        static ColorPlayer vt;
        return &vt;
    }

    int open(const char *url);
    int close();
    int play();
    int pause();
    int resume();
    int stop();
    int set_pos();
    int64_t get_pos();
    int get_play_time_ms();
    int get_video_width();
    int get_video_height();
    int cancel_seek();
    void flush();
    int seek(float position);
    int set_speed();
    int get_speed();
    PlayerInfo *get_player_Instanse();
    virtual ~ColorPlayer();
private:
    ColorPlayer();
    void init_context();
    void deinit_context();
    PlayerInfo *player;
    MasterClock *pMasterClock;
    int bOpened;
};


#endif // COLORPLAYER_H
