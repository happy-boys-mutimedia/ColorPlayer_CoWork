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
#include <QKeyEvent>
#include <QListWidgetItem>

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
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);

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

    void on_Avsync_clicked();

    void trigerMenu(QAction* act);

    void list_doubleclicked(QListWidgetItem* item);

private:
    Ui::Widget *ui;
    int bOpened;
    int bNeedAvsync;
    QListWidget *listWidget;
    int deskWidth;
    int deskHeight;
    int videoWidth;
    int videoHeight;
    QString url;
};

#endif // WIDGET_H
