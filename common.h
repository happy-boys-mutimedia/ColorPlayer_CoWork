#ifndef COMMON_H
#define COMMON_H
#include "stdint.h"

#define FAILED    1
#define SUCCESS   0

enum threadState
{
    THREAD_NONE,
    THREAD_START,
    THREAD_PAUSE,
    THREAD_STOP,
    THREAD_SEEK,
    THREAD_FLUSH
};

enum mediaItem
{
    mediaItem_video,
    mediaItem_audio,
    mediaItem_max
};

typedef void (*pFuncCallback) (mediaItem);
int64_t getCurrentTimeInMs();
void sleepMs_my(unsigned int SleepMs);

#endif // COMMON_H
