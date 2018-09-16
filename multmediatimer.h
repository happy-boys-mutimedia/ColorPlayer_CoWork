#ifndef MULTMEDIATIMER_H
#define MULTMEDIATIMER_H
#include <Windows.h>
#include <MMSystem.h>

class MultMediaTimer
{
public:
    MultMediaTimer();
    virtual ~MultMediaTimer();
    // 回调函数
    //void CALLBACK CallBackFun(UINT wTimerID, UINT msg,DWORD dwUser, DWORD dwl,DWORD dw2);
    void TimerRun();

protected:
    int m_iID = 0;
    // 定时器ID
    MMRESULT m_AudioTimerID;
    MMRESULT m_VideoTimerID;
};

#endif // MULTMEDIATIMER_H
