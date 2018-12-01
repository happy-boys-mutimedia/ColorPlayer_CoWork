#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtWidgets/qwidget.h>
#include <QOpenGLWidget>
#include <QPainter>
#include "colorplayer.h"
#include "messagequeue.h"

class VideoWidget:public QOpenGLWidget
{
public:
    VideoWidget(QWidget *p = NULL);
    void paintEvent(QPaintEvent *e);
    void timerEvent(QTimerEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *m);
    void keyPressEvent(QKeyEvent *event);
    void startVideoWidget(int fps);
    void stopVideoWidget(void);

    message *pMessage;
    virtual ~VideoWidget();
private:
    PlayerInfo *pPlayerInfo;
    uchar *buf;
    QImage *image = NULL;
    QImage *pauseImage = NULL;
    int TimerID = -1;
    QPainter painter;
    int bWindowMini;
    int bDoubleClick;
    int beforeFullScreenWidth;
    int beforeFullScreenHeight;
};

#endif // VIDEOWIDGET_H
