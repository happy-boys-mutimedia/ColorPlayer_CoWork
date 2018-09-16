#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QThread>
class XVideoThread:public QThread
{
public:
    static XVideoThread *Get()
    {
        static XVideoThread vt;
        return &vt;
    }
    XVideoThread();
    void run();
    virtual ~XVideoThread();
};

#endif // VIDEOTHREAD_H
