#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QTimerEvent>
#include <QString>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void openFile(QString name);
    void resizeEvent(QResizeEvent *e);
    void timerEvent(QTimerEvent *e);

    //增加界面菜单栏
    QMenu* menu[10];
    QAction* act[10];
    QMenuBar* menuBar;
    QStatusBar* status;

public slots:
    void on_OpenButton_clicked();

    void on_PlayButton_clicked();

    void on_PlaySlider_sliderPressed();

    void on_PlaySlider_sliderReleased();

    void trigerMenu(QAction* act);

private:
    Ui::Widget *ui;
    int bOpened;

};

#endif // WIDGET_H
