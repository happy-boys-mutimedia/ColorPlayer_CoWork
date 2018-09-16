#ifndef GETHIGHRESOLUTIONSYSTIMER_H
#define GETHIGHRESOLUTIONSYSTIMER_H
#include <windows.h>

class getHighResolutionSysTimer
{
public:

    static getHighResolutionSysTimer *Get()
    {
        static getHighResolutionSysTimer ff;
        return &ff;
    }
    getHighResolutionSysTimer();
    double getSysTimeFre();
    LONGLONG getSysTime();
};

#endif // GETHIGHRESOLUTIONSYSTIMER_H
