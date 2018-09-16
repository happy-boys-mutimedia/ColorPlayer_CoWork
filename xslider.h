#ifndef XSLIDER_H
#define XSLIDER_H

#include <qslider.h>
class XSlider:public QSlider
{
    Q_OBJECT
public:
    XSlider(QWidget *p = NULL);
    virtual ~XSlider();
    void mousePressEvent(QMouseEvent *e);//重载鼠标事件
};


#endif // XSLIDER_H
