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
#include <QEvent>

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
    bWindowMini = 0;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"VideoWidget() error\n";
    }

    TimerID = startTimer(30);
}



void VideoWidget::paintEvent(QPaintEvent *e)
{
    static int w = 0;//记录上一次的界面宽
    static int h = 0;//记录上一次的界面高
    Frame *pFrame = NULL;

    if (pPlayerInfo == NULL)
    {
        pPlayerInfo = ColorPlayer::Get()->get_player_Instanse();
        if (pPlayerInfo == NULL)
        {
            //qDebug()<<"error get_player_Instanse() error";
            return;
        }
    }

    if (pPlayerInfo->playerState == PLAYER_STATE_STOP)
    {
        qDebug()<<"VideoWidget::paintEvent ==> stop";
        return;
    }

    if ((w != width() || h != height()) && image != NULL)
    {
        delete image;
        image = NULL;
    }

    if (image == NULL)
    {
        image = new QImage(width(), height(), QImage::Format_ARGB32);
        w = width();
        h = height();
    }

    pFrame = VideoOutput::Get()->GetFrameFromDisplayQueue(pPlayerInfo);
    if (pFrame)
    {
        XFFmpeg::Get()->PutFrameToConvert(XFFmpeg::Get()->videostreamidx, pFrame->frame);
        XFFmpeg::Get()->ToRGB((char*)image->bits(), width(), height());
    }

    painter.begin(this);
    painter.drawImage(QPoint(0, 0),*image);
    painter.end();

    if (pFrame)
        VideoOutput::Get()->receiveFrametoDisplayQueue(pFrame);
}

void VideoWidget::timerEvent(QTimerEvent *e)
{
    MessageCmd_t MsgCmd;
    Frame *pFrame = NULL;

    if (pMessage->message_dequeue(&MsgCmd) == SUCCESS)
    {
        if (MESSAGE_CMD_WINDOW_MINMiZED == MsgCmd.cmd)
        {
            qDebug()<<"VideoWidget::timerEvent get cmd MESSAGE_CMD_WINDOW_MINMiZED";
            bWindowMini = 1;
        }
        else if (MESSAGE_CMD_WINDOW_RESUME == MsgCmd.cmd)
        {
            qDebug()<<"VideoWidget::timerEvent get cmd MESSAGE_CMD_WINDOW_RESUME";
            bWindowMini = 0;
        }
    }

    if (!bWindowMini)
    {
        this->update();
    }
    else if (pPlayerInfo)
    {
        pFrame = VideoOutput::Get()->GetFrameFromDisplayQueue(pPlayerInfo);
        if (pFrame)
            VideoOutput::Get()->receiveFrametoDisplayQueue(pFrame);
    }
}


VideoWidget::~VideoWidget()
{
    if (TimerID != -1)
    {
        killTimer(TimerID);
    }

    if (image)
    {
        delete image;
        image = NULL;
    }

    if (pMessage)
    {
        delete pMessage;
    }
}
