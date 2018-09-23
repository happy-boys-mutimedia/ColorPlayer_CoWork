#include "masterclock.h"
#include "common.h"
#include <QDebug>

MasterClock::MasterClock()
{
    running = 0;
    audio_clock = -1;
    pause_clock = -1;
    base_clock = -1;
    last_apts = -1;
    clock_drift = -1;
    play_start_time = -1;
    clock_source = AUDIO_MASTER;
    video_latency = 0;
}

int MasterClock::open(int clockSource)
{
    clock_source = clockSource;
    running = 1;
    return SUCCESS;
}

int MasterClock::close()
{
    running = 0;
    audio_clock = -1;
    pause_clock = -1;
    base_clock = -1;
    last_apts = -1;
    clock_drift = -1;
    play_start_time = -1;
    clock_source = AUDIO_MASTER;
    video_latency = 0;
    return SUCCESS;
}

int64_t MasterClock::convert_to_system_time(int64_t clock)
{
    //qDebug()<<"convert_to_system_time clock:"<<clock<<"base_clock "<<base_clock<<"play_start_time "<<play_start_time;
    return clock - base_clock + play_start_time;
}

int64_t MasterClock::get_master_clock()
{
    int64_t master_clock;
    //qDebug()<<"get_master_clock IN";
    if (!running)
    {
        return pause_clock - video_latency;
    }
    else if (clock_source == SYSTEM_MASTER)
    {
        //todo
        //return system_clock;
    }
    else if (clock_source == AUDIO_MASTER)
    {
        master_clock = convert_to_system_time(audio_clock);
        //qDebug()<<"master_clock "<<master_clock;
        return master_clock;
    }
}

int64_t MasterClock::get_audio_delay()
{
    //todo
    return 0;
}

int MasterClock::run()
{
    //todo
    running = 1;
    return SUCCESS;
}

int MasterClock::pause()
{
    pause_clock = get_master_clock();
    running = 0;
    return SUCCESS;
}

void MasterClock::flush()
{
    audio_clock = -1;
    pause_clock = -1;
    base_clock = -1;
    last_apts = -1;
    clock_drift = -1;
    play_start_time = -1;
    clock_source = AUDIO_MASTER;
    video_latency = 0;
}

int64_t MasterClock::get_audio_clock()
{
    return audio_clock;
}

int MasterClock::set_audio_clock(int64_t AudioClock)
{
    mutex.lock();
    audio_clock = AudioClock;
    //qDebug()<<"set_audio_clock "<<AudioClock;
    mutex.unlock();
    return SUCCESS;
}

int MasterClock::get_clock_base()
{
    return base_clock;
}

int MasterClock::set_clock_base(int64_t clock)
{
    base_clock = clock;
    clock_drift = 0;

    pause_clock = play_start_time = getCurrentTimeInMs();
    //qDebug()<<"set_clock_base base_clock:"<<base_clock<<"play_start_time "<<play_start_time;
    return SUCCESS;
}

int MasterClock::get_time()
{
    if (base_clock == -1)
        return 0;

    return get_master_clock()
         - play_start_time
         + get_clock_base();
}

void MasterClock::switch_source(int source)
{
    clock_source = source;
}

MasterClock::~MasterClock()
{
    running = 0;
    audio_clock = -1;
    pause_clock = -1;
    base_clock = -1;
    last_apts = -1;
    clock_drift = -1;
    play_start_time = -1;
    clock_source = AUDIO_MASTER;
    video_latency = 0;
}
