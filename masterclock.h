#ifndef MASTERCLOCK_H
#define MASTERCLOCK_H
#include <QMutex>
#include "stdint.h"

enum {
    AUDIO_MASTER = 0,
    VIDEO_MASTER,
    SYSTEM_MASTER
};

class MasterClock
{
public:
    MasterClock();

    int open(int clockSource);
    int close();
    int64_t convert_to_system_time(int64_t clock);
    int64_t get_master_clock();
    int64_t get_audio_delay();
    int run();
    int pause();
    void flush();
    int64_t get_audio_clock();
    int set_audio_clock(int64_t AudioClock);
    int get_clock_base();
    int set_clock_base(int64_t clock);
    int get_time();
    void switch_source(int source);
    virtual ~MasterClock();
private:
    int      running;          /* whether clock is running or stopped/paused */
    int64_t  audio_clock;      /* the timestamp of the last delivered audio chunk */
    int64_t  pause_clock;      /* the clock value when playback paused */
    int64_t  base_clock;       /* the initial clock value */
    int64_t  last_apts;        /* the last time we update drift value */
    int64_t  clock_drift;      /* the difference between system clock and audio clock */
    int64_t  play_start_time;  /* the system time when clock base is set */
    int      clock_source;     /* one of MASTER_SOURCE, SYSTEM_SOURCE, or AUDIO_SOURCE */
    int      video_latency;    /* the latency from video flip to displaying on screen */
    QMutex mutex;
};


#endif // MASTERCLOCK_H
