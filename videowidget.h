#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtWidgets/qwidget.h>
#include <QOpenGLWidget>
#include "colorplayer.h"
#include "messagequeue.h"

class VideoWidget:public QOpenGLWidget
{
public:
    VideoWidget(QWidget *p = NULL);
    void paintEvent(QPaintEvent *e);
    void timerEvent(QTimerEvent *e);

    virtual ~VideoWidget();
private:
    PlayerInfo *pPlayerInfo;
    message *pMessage;
    uchar *buf;
};

#endif // VIDEOWIDGET_H
