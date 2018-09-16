#include "gethighresolutionsystimer.h"


getHighResolutionSysTimer::getHighResolutionSysTimer()
{

}

double getHighResolutionSysTimer::getSysTimeFre()
{
    LARGE_INTEGER litmp;
    double dfFreq;
    //获得CPU计时器的时钟频率
    QueryPerformanceFrequency(&litmp);//取得高精度运行计数器的频率f,单位是每秒多少次（n/s），
    dfFreq = (double)litmp.QuadPart;

    return dfFreq;
}

LONGLONG getSysTime()
{
    LONGLONG Qpart;
    LARGE_INTEGER litmp;

    QueryPerformanceCounter(&litmp);//取得高精度运行计数器的数值
    Qpart = litmp.QuadPart; //获取滴答值

    return Qpart;
}






