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

    menu[0] = new QMenu("文件");
    menu[0]->addAction("编辑");
    menu[0]->addAction("查看");
    menu[0]->addAction("工具");

    act[0] = new QAction("新建",this);
    act[0]->setShortcut(Qt::CTRL | Qt::Key_A );
    act[0]->setStatusTip("new menu");

    act[1] = new QAction("打开",this);
    act[1]->setCheckable(true);

    menu[1] = new QMenu("保存");
    menu[1]->addAction(act[0]);
    menu[1]->addAction(act[1]);

    menu[2] = new QMenu("打印");
    menu[2]->addAction("打印设置");
    menu[2]->addMenu(menu[1]);

    menuBar = new QMenuBar(this);
    menuBar->addMenu(menu[0]);
    menuBar->addMenu(menu[2]);
    menuBar->setGeometry(0,0,this->width(),30);

    connect(menuBar,SIGNAL(triggered(QAction*)),this,SLOT(trigerMenu(QAction*)));

    bOpened = 0;
}

Widget::~Widget()
{
    delete menu[0];
    delete menu[1];
    delete menu[2];
    delete act[0];
    delete act[1];
    delete menuBar;
    delete ui;

    //restore window sleep
    SetThreadExecutionState(ES_CONTINUOUS);
}

void Widget::on_OpenButton_clicked()
{
    QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("please choose file"));
    openFile(name);
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

    if (isPlay)
    {
        //pause
        ui->PlayButton->setStyleSheet("QPushButton{color:blue}");
        //ui->PlayButton->setStyleSheet(PAUSE);
    }
    else
    {
        //play
        ui->PlayButton->setStyleSheet("QPushButton{color:red}");
        //ui->PlayButton->setStyleSheet(PLAY);
    }
}

void Widget::on_PlaySlider_sliderPressed()
{
    isPressedSlider = true;
}

void Widget::on_PlaySlider_sliderReleased()
{
    printf("slider_released\n");
    isPressedSlider = false;
    float pos = 0;
    pos = (float)ui->PlaySlider->value() / (float)(ui->PlaySlider->maximum() + 1);
    ColorPlayer::Get()->seek(pos);
}

void Widget::resizeEvent(QResizeEvent *e)
{
    this->menuBar->resize(this->width(),25);
    ui->openGLWidget->resize(size());
    ui->OpenButton->move(5, this->height()-40);
    //ui->OpenButton->resize(this->width()/10, this->height()/10);
    ui->PlayButton->move(70, this->height()-40);
    ui->PlaySlider->move(5,this->height() - 15);
    ui->PlaySlider->resize(this->width(), ui->PlaySlider->height());
    ui->PlayTime->move(5, this->height() - 60);
    ui->TotalTime->move(70, this->height() - 60);
    ui->SP->move(70, this->height() - 60);
}

void Widget::timerEvent(QTimerEvent *e)
{
    char buf[1024] = {0};

    int playTime = ColorPlayer::Get()->get_pos();

    int hour = (playTime / 1000) / (60 * 60);
    int min = ((playTime / 1000) % (60 * 60)) / 60;
    int sec = (playTime / 1000) % 60;
    sprintf(buf, "%02d:%02d:%02d", hour, min, sec);
    ui->PlayTime->setText(buf);

    if (ColorPlayer::Get()->get_play_time_ms() > 0)
    {
        float rate = (float)playTime / (float)ColorPlayer::Get()->get_play_time_ms();
        if (!isPressedSlider)/*如果没有按下进度条才显示*/
        ui->PlaySlider->setValue(rate * 1000);
    }
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
    int deskWidth = clientRect.width();
    int deskHeight = clientRect.height();
    qDebug()<<"w "<<deskWidth<<"h "<<deskHeight;

    //按视频大小调整播放屏幕大小
    int videoWidth = ColorPlayer::Get()->get_video_width();
    int videoHeight = ColorPlayer::Get()->get_video_height();
    if (videoWidth <= deskWidth && videoHeight <= deskHeight)
    {
        this->move((deskWidth - videoWidth)/2, 0);
        this->resize(videoWidth, videoHeight);
    }
    else
    {
        this->move(0,0);
        this->resize(deskWidth, deskHeight - 40);
    }
    qDebug()<<"=======================open successful!!!!!!!=========================";
    isPlay = false;
}

void Widget::trigerMenu(QAction* act)
{
    if(act->text() == "新建")
    {
        qDebug()<<"press down";
    }
}


