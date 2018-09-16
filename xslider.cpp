#include "xslider.h"
#include <QMouseEvent>
#include <QDebug>
XSlider::XSlider(QWidget *p) :QSlider(p)
{
}

void XSlider::mousePressEvent(QMouseEvent *e)
{
    int value = ((float)e->pos().x() / (float)this->width()) * this->maximum() + 1;
    this->setValue(value);
    QSlider::mousePressEvent(e);
}

XSlider::~XSlider()
{
}
