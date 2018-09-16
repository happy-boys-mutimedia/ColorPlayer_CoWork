#include "windows_timer.h"
#include <windows.h>

static void caculate()
{
    for(int i=0;i<32323;i++)
    {
        for(int j=0;j<32323;j++) ;
    }
}

windows_timer::windows_timer()
{
    LARGE_INTEGER litmp;
    LONGLONG Qpart1,Qpart2,Useingtime;
    double dfMinus,dfFreq,dfTime;

    //获得CPU计时器的时钟频率
    QueryPerformanceFrequency(&litmp);//取得高精度运行计数器的频率f,单位是每秒多少次（n/s），
    dfFreq = (double)litmp.QuadPart;

    QueryPerformanceCounter(&litmp);//取得高精度运行计数器的数值
    Qpart1 = litmp.QuadPart; //开始计时

    caculate(); //待测试的计算函数等
    QueryPerformanceCounter(&litmp);//取得高精度运行计数器的数值
    Qpart2 = litmp.QuadPart; //终止计时

    dfMinus = (double)(Qpart2 - Qpart1);//计算计数器值
    dfTime = dfMinus / dfFreq;//获得对应时间，云锡摇床。单位为秒,可以乘精确到微秒级（us）
    Useingtime = dfTime * 1000000;
}



