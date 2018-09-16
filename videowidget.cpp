#include "VideoWidget.h"
#include <QPainter>
#include <QTime>
#include "ffmpeg.h"
#include <QDebug>
#include "audioplay_sdl2.h"
#include "audiodecodethread.h"
#include "videodecodethread.h"
#include "videooutput.h"
#include "colorplayer.h"


QImage *image = NULL;
int TimerID = -1;
QPainter painter;

static int getTimeInUs()
{
    QTime current_time =QTime::currentTime();
    int hour = current_time.hour();//当前的小时
    int minute = current_time.minute();//当前的分
    int second = current_time.second();//当前的秒
    int msec = current_time.msec();//当前的毫秒

    return (hour * 60 * 60 * 1000000 + minute * 60 * 1000000 + second * 1000000 + msec * 1000);
}

VideoWidget::VideoWidget(QWidget *p) : QOpenGLWidget(p)
{
    pPlayerInfo = NULL;
    buf = NULL;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"VideoWidget() error\n";
    }

    TimerID = startTimer(10);
}

Frame *pPauseFrame = NULL;

void VideoWidget::paintEvent(QPaintEvent *e)
{
    static int w = 0;//记录上一次的界面宽
    static int h = 0;//记录上一次的界面高

    if (pPlayerInfo == NULL)
    {
        pPlayerInfo = ColorPlayer::Get()->get_player_Instanse();
        if (pPlayerInfo == NULL)
        {
            //qDebug()<<"error get_player_Instanse() error";
            return;
        }
    }

    if ((w != width() || h != height()) && image != NULL)
    {
        delete image->bits();
        delete image;
        if (buf)
        {
            qDebug()<<"buf :"<<buf;
            //free(buf);//????会失败
            //delete []buf;
        }
        image = NULL;
        buf = NULL;
    }

    if (image == NULL && buf == NULL)
    {
        buf = new uchar[width() * height() * 4];
        //buf = (uchar *)malloc(width() * height() * 4 *4);//分配了没有释放资源
        //qDebug()<<"malloc buf :"<<buf;
        image = new QImage(buf, width(), height(), QImage::Format_ARGB32);
        w = width();
        h = height();
    }

    Frame *pFrame = NULL;
    pFrame = VideoOutput::Get()->GetFrameFromDisplayQueue(pPlayerInfo);
    if (!pFrame)
        return;

    XFFmpeg::Get()->PutFrameToConvert(XFFmpeg::Get()->videostreamidx, pFrame->frame);
    XFFmpeg::Get()->ToRGB((char*)image->bits(), width(), height());

    painter.begin(this);
    painter.drawImage(QPoint(0, 0),*image);
    painter.end();

    VideoOutput::Get()->receiveFrametoDisplayQueue(pFrame);
}

void VideoWidget::timerEvent(QTimerEvent *e)
{
    this->update();
}


VideoWidget::~VideoWidget()
{
    if (TimerID != -1)
    {
        killTimer(TimerID);
    }

    if (image)
    {
        delete image->bits();
        delete image;
        image = NULL;
    }

    if (pMessage)
    {
        delete pMessage;
    }

    if (buf)
    {
        free((void *)buf);//why?
    }
}
