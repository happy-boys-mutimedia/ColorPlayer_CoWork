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
#include <QKeyEvent>

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
    bDoubleClick = 0;
    beforeFullScreenWidth = 1280;
    beforeFullScreenHeight = 720;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"VideoWidget() error\n";
    }
}

static int w = 0;//记录上一次的界面宽
static int h = 0;//记录上一次的界面高
static int cnt = 0;
static int lastTime = 0;
void VideoWidget::paintEvent(QPaintEvent *e)
{
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

    if (pPlayerInfo->playerState != PLAYER_STATE_START && pPlayerInfo->playerState != PLAYER_STATE_PAUSE)
    {
        //qDebug()<<"VideoWidget::paintEvent ==> stop";
        return;
    }

    if (lastTime == 0)
    {
        lastTime = getCurrentTimeInMs();
    }
    else
    {
        if (getCurrentTimeInMs() - lastTime >= 1000)
        {
            //qDebug()<<"VideoWidget::paintEvent ==> cnt = "<<cnt;
            cnt = 0;
            lastTime = getCurrentTimeInMs();
        }
        else
        {
            cnt++;
        }
    }

    //qDebug()<<"VideoWidget::paintEvent IN ==>"<<getCurrentTimeInMs();
    if ((w != width() || h != height()) && image != NULL)
    {
        qDebug()<<"VideoWidget::paintEvent ==> w "<<w<<"h "<<h<<"==> new width "<<width()<<"height "<<height();
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

        painter.begin(this);
        painter.drawImage(QPoint(0, 0),*image);
        painter.end();

        VideoOutput::Get()->receiveFrametoDisplayQueue(pFrame);
    }
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

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *m)
{
    qDebug()<<"VideoWidget::mouseDoubleClickEvent IN";

    bDoubleClick = !bDoubleClick;
    if (bDoubleClick)
    {
        beforeFullScreenWidth = width();
        beforeFullScreenHeight = height();
        setWindowFlags(Qt::Dialog);
        showFullScreen();
    }
    else
    {
        setWindowFlags(Qt::SubWindow);
        showNormal();
        resize(beforeFullScreenWidth, beforeFullScreenHeight);
        move(0,0);
    }
}

void VideoWidget::keyPressEvent(QKeyEvent *event)
{
    qDebug()<<"VideoWidget::keyPressEvent:"<<event->key();
    if (event->key() == Qt::Key_Escape)
    {
        if (bDoubleClick)
        {
            qDebug()<<"VideoWidget ==> Qt::Key_Escape "<<bDoubleClick;
            setWindowFlags(Qt::SubWindow);
            showNormal();
            resize(beforeFullScreenWidth, beforeFullScreenHeight);
            move(0,0);
            bDoubleClick = !bDoubleClick;
        }
    }
}

void VideoWidget::startVideoWidget(int fps)
{
    qDebug()<<"VideoWidget::startVideoWidget fps =>"<<fps;
    int timerMs = 30;

    if (fps)
        timerMs = 1000/fps;

    qDebug()<<"fps "<<fps<<"timerMs "<<timerMs<<"ms";
    TimerID = startTimer(timerMs, Qt::PreciseTimer);
}

void VideoWidget::stopVideoWidget(void)
{
    qDebug()<<"VideoWidget::stopVideoWidget IN";
    if (TimerID != -1)
    {
        killTimer(TimerID);
        TimerID = -1;
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
