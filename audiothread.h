#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QThread>
class audiothread:public QThread
{
public:
    static audiothread *Get()
    {
        static audiothread vt;
        return &vt;
    }
    void run();
    audiothread();
    virtual ~audiothread();
};

#endif // AUDIOTHREAD_H
