#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopWidget>
#include "xslider.h"
#include "ffmpeg.h"
#include "audioplay_sdl2.h"
#include "colorplayer.h"
#include <QDebug>
#include <QInputDialog>
#include <QDir>
#include <QListWidget>
#include <QMimeData>
#include <QMovie>


static bool isPressedSlider = false;
static bool isPlay = true;
//#define PAUSE "border-image: url(E:/QT5.6.2/OnlyQt_myplayer/ico/stop.png);"
//#define PLAY "border-image: url(E:/QT5.6.2/OnlyQt_myplayer/ico/play.png);"

#define PAUSE "border-image: url(/icon/stop.png);"
#define PLAY "border-image: url(/icon/play.png);"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    startTimer(20);

    status = new QStatusBar(this);

    menu[0] = new QMenu("Files");

    act[0] = new QAction("open",this);
    act[0]->setShortcut(QKeySequence::Open);
    act[0]->setStatusTip(tr("open media file"));
    act[1] = new QAction("open network stream",this);
    act[2] = new QAction("close",this);
    act[2]->setShortcut(QKeySequence::Quit);
    act[2]->setStatusTip(tr("exit"));
    menu[0]->addAction(act[0]);
    menu[0]->addAction(act[1]);
    menu[0]->addAction(act[2]);

    menu[1] = new QMenu("Action");
    menu[1]->addAction("pause/play");
    menu[1]->addAction("previous media file");
    menu[1]->addAction("next media file");
    menu[1]->addAction("avsync/no avsync");

    menu[2] = new QMenu("PlayList");
    menu[2]->addAction("list");

    menu[3] = new QMenu("MultiPlay");
    menu[3]->addAction("1.0");
    menu[3]->addAction("1.25");
    menu[3]->addAction("1.5");
    menu[3]->addAction("1.75");
    menu[3]->addAction("2.0");

    menu[4] = new QMenu("Help");
    menu[4]->addAction("aboutMe");

    menuBar = new QMenuBar(this);
    menuBar->addMenu(menu[0]);
    menuBar->addMenu(menu[1]);
    menuBar->addMenu(menu[2]);
    menuBar->addMenu(menu[3]);
    menuBar->addMenu(menu[4]);
    menuBar->setGeometry(0,0,this->width(),30);

    connect(menuBar,SIGNAL(triggered(QAction*)),this,SLOT(trigerMenu(QAction*)));

    //allow drop envent
    ui->openGLWidget->setAcceptDrops(false);
    setAcceptDrops(true);

    ui->AudioSlider->setValue(0.5 * ui->AudioSlider->maximum());

    //无边框
    //setWindowFlags(Qt::FramelessWindowHint);

    bOpened = 0;
    bNeedAvsync = 1;
    listWidget = NULL;
    bNetworkStream = 0;
}

Widget::~Widget()
{
    qDebug()<<"Widget::~Widget()";
    delete menu[0];
    delete menu[1];
    delete menu[2];
    delete menu[3];
    delete menu[4];
    delete act[0];
    delete act[1];
    delete menuBar;
    delete status;
    delete ui;

    if (listWidget)
    {
        for(int i = 0; i < listWidget->count(); i++)
        {
            QListWidgetItem *item = NULL;
            item = listWidget->takeItem(i);
            delete item;
        }
        delete listWidget;
    }

    //restore window sleep
    SetThreadExecutionState(ES_CONTINUOUS);
}

void Widget::getMultiList(QString url)
{
    QFileInfo urlFileInfo(url);
    QDir dir = urlFileInfo.absoluteDir();
    if (!dir.exists())
    {
        qDebug()<<"dir no exits";
        return;
    }
    qDebug()<<"dir ==>"<<dir.absolutePath();

    QStringList filters;
    filters << "*.MKV" << "*.mkv";
    filters << "*.RMVB" << "*.rmvb"<<"*.rm";
    filters << "*.MOV" << "*.mov";
    filters << "*.AVI" << "*.avi";
    filters << "*.MP4" << "*.mp4";
    filters << "*.TS" << "*.ts";
    filters << "*.DIVX" << "*.divx";
    filters << "*.ASF" << "*.asf";
    filters << "*.MPG" << "*.mpg";
    filters << "*.FLV" << "*.flv";
    filters << "*.3GP" << "*.3gp";
    filters << "*.MPEG" << "*.mpeg";
    filters << "*.WMV" << "*.wmv";
    filters << "*.MPG" << "*.mpg";
    filters << "*.aac" << "*.m4a";
    filters << "*.flac" << "*.m4a";
    filters << "*.mp3" << "*.ogg";
    filters << "*.mid" << "*.asx";
    filters << "*.ra" << "*.wma";

    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name);
    dir.setNameFilters(filters);

    QFileInfoList list = dir.entryInfoList();
    listWidget = new QListWidget();
    for (int i = 0; i < list.size(); i++)
    {
        QFileInfo fileInfo = list.at(i);
        qDebug()<<"list of names ==> "<<i<<":"<<fileInfo.fileName();
        new QListWidgetItem(fileInfo.absoluteFilePath(),listWidget);
    }

    connect(listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(list_doubleclicked(QListWidgetItem*)));
}

void Widget::on_OpenButton_clicked()
{
    url = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("please choose file"));
    openFile(url);
    bNetworkStream = 0;
    getMultiList(url);
}

void Widget::on_PlayButton_clicked()
{
    isPlay = !isPlay;
    if (isPlay)
    {
        ColorPlayer::Get()->pause();
    }
    else
    {
        ColorPlayer::Get()->resume();
    }
}

void Widget::on_Avsync_clicked()
{
    bNeedAvsync = !bNeedAvsync;
    if (bNeedAvsync)
    {
        ColorPlayer::Get()->need_avsync();
    }
    else
    {
        ColorPlayer::Get()->cancel_avsync();
    }
}

void Widget::on_PlaySlider_sliderPressed()
{
    isPressedSlider = true;
}

void Widget::on_PlaySlider_sliderReleased()
{
    qDebug()<<"on_PlaySlider_sliderReleased slider_released IN";
    isPressedSlider = false;
    float pos = 0;
    pos = (float)ui->PlaySlider->value() / (float)(ui->PlaySlider->maximum() + 1);
    qDebug()<<"pos "<<pos;
    if (!bNetworkStream)
        ColorPlayer::Get()->seek(pos);
}

void Widget::AudioSlider_pressed()
{
    qDebug()<<"Widget::AudioSlider_pressed IN";
}

void Widget::AudioSlider_released()
{
    qDebug()<<"Widget::AudioSlider_released IN";
    float pos = 0;
    pos = (float)ui->AudioSlider->value() / (float)(ui->AudioSlider->maximum() + 1);
    qDebug()<<"pos ==> "<<pos;
    ColorPlayer::Get()->set_volume(pos);
}

void Widget::resizeEvent(QResizeEvent *e)
{
    qDebug()<<"Widget::resizeEvent IN"<<"w "<<this->width()<<"h "<<this->height();

    this->menuBar->resize(this->width(),25);
    ui->openGLWidget->resize(this->width(), this->height() - 20);
    ui->PlaySlider->move(48,this->height() - 19);
    ui->PlaySlider->resize(this->width() - 96 - ui->AudioSlider->width(), ui->PlaySlider->height());
    ui->AudioSlider->move(96 + ui->PlaySlider->width(),this->height() - 19);
    ui->PlayTime->move(0, this->height() - 16);
    ui->TotalTime->move(this->width() - 48 - ui->AudioSlider->width(), this->height() - 16);
}

void Widget::timerEvent(QTimerEvent *e)
{
    char buf[1024] = {0};

    int playTime = ColorPlayer::Get()->get_pos();
    if (playTime != -1)
    {
        int hour = (playTime / 1000) / (60 * 60);
        int min = ((playTime / 1000) % (60 * 60)) / 60;
        int sec = (playTime / 1000) % 60;
        sprintf(buf, "%02d:%02d:%02d", hour, min, sec);
        ui->PlayTime->setText(buf);

        if (ColorPlayer::Get()->get_play_time_ms() > 0)
        {
            float rate = (float)playTime / (float)ColorPlayer::Get()->get_play_time_ms();
            if (!isPressedSlider)/*如果没有按下进度条才显示*/
            ui->PlaySlider->setValue(rate * ui->PlaySlider->maximum());
        }
    }
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    qDebug()<<"Qt::Key_xxx:"<<event->key();
    if(event->key() == Qt::Key_Space)
    {
        on_PlayButton_clicked();
    }
    else if(event->key() == Qt::Key_Q)
    {
        qDebug()<<"Qt::Key_Q";
        float CurPos = 0;
        CurPos = (float)ui->PlaySlider->value() / (float)(ui->PlaySlider->maximum() + 1);
        CurPos = CurPos + 0.005;
        if (CurPos > 1.0)
        {
            CurPos = 1.0;
        }
        ColorPlayer::Get()->seek(CurPos);
    }
    else if(event->key() == Qt::Key_H)
    {
        qDebug()<<"Qt::Key_H";
        float CurPos = 0;
        CurPos = (float)ui->PlaySlider->value() / (float)(ui->PlaySlider->maximum() + 1);
        CurPos = CurPos - 0.005;
        if (CurPos < 0)
        {
            CurPos = 0;
        }
        ColorPlayer::Get()->seek(CurPos);
    }
}

void Widget::closeEvent(QCloseEvent *event)
{
    qDebug()<<"Widget::closeEvent IN";
    if (listWidget)
        listWidget->hide();

    ColorPlayer::Get()->stop();
}

void Widget::openFile(QString name)
{
    if (name.isEmpty())
    {
        return;
    }
    this->setWindowTitle(name);

    if (!bOpened)
    {
        if (ColorPlayer::Get()->open(name.toLocal8Bit()) == SUCCESS)
        {
            bOpened = 1;
            ColorPlayer::Get()->play();
        }
        else
        {
            QMessageBox::information(this, "err", "file open error");
        }
    }
    else
    {
        ColorPlayer::Get()->close();
        ui->openGLWidget->stopVideoWidget();
        if (ColorPlayer::Get()->open(name.toLocal8Bit()) == SUCCESS)
        {
            bOpened = 1;
            ColorPlayer::Get()->play();
        }
        else
        {
            QMessageBox::information(this, "err", "file open error");
        }
    }

    //stop windows sleep
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);

    int totalMs = ColorPlayer::Get()->get_play_time_ms();
    char buf[1024] = { 0 };
    int hour = (totalMs / 1000) / (60 * 60);
    int min = ((totalMs / 1000) % (60 * 60)) / 60;
    int sec = (totalMs / 1000) % 60;
    sprintf(buf, "%02d:%02d:%02d", hour, min, sec);
    ui->TotalTime->setText(buf);

    //获取屏幕大小
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect clientRect = desktopWidget->availableGeometry();
    deskWidth = clientRect.width();
    deskHeight = clientRect.height();
    qDebug()<<"w "<<deskWidth<<"h "<<deskHeight;

    //按视频大小调整播放屏幕大小
    videoWidth = ColorPlayer::Get()->get_video_width();
    videoHeight = ColorPlayer::Get()->get_video_height();
    if (videoWidth <= deskWidth && videoHeight <= deskHeight)
    {
        this->move(0 , 0);//(deskWidth - videoWidth)/2
        this->resize(videoWidth, videoHeight);
    }
    else
    {
        this->move(0,0);
        this->resize(deskWidth, deskHeight - 40);
    }
    int fps = ColorPlayer::Get()->get_fps();
    ui->openGLWidget->startVideoWidget(fps);
    qDebug()<<"=======================open successful!!!!!!!=========================";
    isPlay = false;

}

void Widget::trigerMenu(QAction* act)
{
    if(act->text() == "open")
    {
        qDebug()<<"press down";
        on_OpenButton_clicked();
        return;
    }

    if(act->text() == "pause/play")
    {
        on_PlayButton_clicked();
        return;
    }

    if(act->text() == "previous media file")
    {
        int currenRow = 0;

        if (listWidget)
        {
            for (int i = 0; i < listWidget->count(); i++)
            {
                QListWidgetItem * item = listWidget->item(i);
                QString name = item->text();

                if (!name.compare(url))
                {
                    currenRow = i;
                    if (i + 1 == listWidget->count())
                    {
                        i = 0;
                    }
                    else
                    {
                        i = i + 1;
                    }
                    item = listWidget->item(i);
                    if (item)
                    {
                        name = item->text();
                    }
                    else
                    {
                        return;
                    }
                    url = name;
                    openFile(name);
                    bNetworkStream = 0;
                    qDebug()<<"open ==> name -----> "<<item->text();
                }
            }
        }
        return;
    }

    if(act->text() == "next media file")
    {
        int currenRow = 0;
        if (listWidget)
        {
            for (int i = 0; i < listWidget->count(); i++)
            {
                QListWidgetItem * item = listWidget->item(i);
                QString name = item->text();

                if (!name.compare(url))
                {
                    currenRow = i;
                    if (i - 1 == -1)
                    {
                        i = listWidget->count() - 1;
                    }
                    else
                    {
                        i = i - 1;
                    }
                    item = listWidget->item(i);
                    if (item)
                    {
                        name = item->text();
                    }
                    else
                    {
                        return;
                    }
                    url = name;
                    openFile(name);
                    bNetworkStream = 0;
                    qDebug()<<"open ==> name -----> "<<item->text();
                }
            }
        }
        return;
    }

    if(act->text() == "close")
    {
        ColorPlayer::Get()->close();
        ui->openGLWidget->stopVideoWidget();
        bNetworkStream = 0;
        return;
    }

    if(act->text() == "open network stream")
    {
        QString url = QInputDialog::getText(this, "network URL", "Please inter URL", QLineEdit::Normal,
                                             "http://dlhls.cdn.zhanqi.tv/zqlive/49427_jmACJ.m3u8");
        ColorPlayer::Get()->set_networkStreamFlag(true);
        openFile(url);
        bNetworkStream = 1;
        return;
    }

    if(act->text() == "avsync/no avsync")
    {
        on_Avsync_clicked();
        return;
    }

    if(act->text() == "aboutMe")
    {
        QMessageBox::about(this, "about me", "Created by QiujieLu \nmail:1511248339@qq.com \ngithub:https://github.com/happy-boys-mutimedia/ColorPlayer_CoWork");
        return;
    }

    if(act->text() == "list")
    {
        int pos_x = 0;
        if (listWidget)
        {
            listWidget->resize(300, 600);
            if ((deskWidth + videoWidth)/2 >= deskWidth)
            {
                pos_x = deskWidth/2;
            }
            else
            {
                pos_x = (deskWidth + videoWidth)/2;
            }
            listWidget->move(pos_x,0);
            listWidget->show();
        }
        else
        {
            QMessageBox::information(this, "err", "list is empty!");
        }
        return;
    }

    if(act->text() == "1.0" || act->text() == "1.25" || act->text() == "1.5" ||
            act->text() == "1.75" || act->text() == "2.0")
    {
        float multiple = 0;

        bool ok = 0;
        multiple = act->text().toFloat(&ok);
        if (ok)
        {
            qDebug()<<"multiple = "<<multiple;
            ColorPlayer::Get()->multiplePlay(multiple);
        }
        return;
    }
}

void Widget::changeEvent(QEvent *e)
{
    qDebug()<<"Widget::changeEvent IN";
    if(e->type() == QEvent::WindowStateChange)
    {
        if(windowState() & Qt::WindowMinimized)
        {
            MessageCmd_t MsgCmd;
            qDebug()<< "widget send windowMinmized cmd!!";
            MsgCmd.cmd = MESSAGE_CMD_WINDOW_MINMiZED;
            MsgCmd.cmdType = MESSAGE_CMD_QUEUE;
            ui->openGLWidget->pMessage->message_queue(MsgCmd);
        }
        else
        {
            MessageCmd_t MsgCmd;
            qDebug()<< "widget send windowMinmized cmd!!";
            MsgCmd.cmd = MESSAGE_CMD_WINDOW_RESUME;
            MsgCmd.cmdType = MESSAGE_CMD_QUEUE;
            ui->openGLWidget->pMessage->message_queue(MsgCmd);
        }
    }
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug()<< "Widget::dragEnterEvent IN!";
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void Widget::dropEvent(QDropEvent *event)
{
    qDebug()<< "Widget::dropEvent IN!";
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
       return;
    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return;
    qDebug()<< "fileName = "<<fileName;
    openFile(fileName);
    bNetworkStream = 0;
    getMultiList(fileName);
}

void Widget::list_doubleclicked(QListWidgetItem* item)
{
    qDebug()<<"list_doubleclicked IN";
    qDebug()<<"text:"<<item->text();
    QString name = item->text();
    url = name;
    openFile(name);
    bNetworkStream = 0;

    listWidget->hide();
}

