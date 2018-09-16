#ifndef COLORPLAYER_UI_H
#define COLORPLAYER_UI_H

#endif // COLORPLAYER_UI_H

#include <QWidget>
#include "widget.h"

class ColorPlayer : public QWidget
{
    Q_OBJECT

public:
    ColorPlayer(QWidget *parent = Q_NULLPTR);
    void resizeEvent(QResizeEvent *e);
    void timerEvent(QTimerEvent *e);
    void openFile(QString name);
public slots:
    void open();
    void play();
    void slider_press();
    void slider_released();

private:
    Ui::XplayerClass ui;
};
