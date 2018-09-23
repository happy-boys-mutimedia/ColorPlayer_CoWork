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

int64_t getCurrentTimeInMs();

#endif // COMMON_H
