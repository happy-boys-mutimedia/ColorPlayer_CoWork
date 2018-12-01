/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include "videowidget.h"
#include "xslider.h"

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    VideoWidget *openGLWidget;
    XSlider *PlaySlider;
    QLabel *PlayTime;
    QLabel *TotalTime;
    XSlider *AudioSlider;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(459, 352);
        openGLWidget = new VideoWidget(Widget);
        openGLWidget->setObjectName(QStringLiteral("openGLWidget"));
        openGLWidget->setGeometry(QRect(0, 0, 461, 271));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(openGLWidget->sizePolicy().hasHeightForWidth());
        openGLWidget->setSizePolicy(sizePolicy);
        openGLWidget->setMaximumSize(QSize(1920, 1080));
        PlaySlider = new XSlider(Widget);
        PlaySlider->setObjectName(QStringLiteral("PlaySlider"));
        PlaySlider->setGeometry(QRect(46, 320, 311, 22));
        PlaySlider->setMaximum(999);
        PlaySlider->setPageStep(100);
        PlaySlider->setOrientation(Qt::Horizontal);
        PlayTime = new QLabel(Widget);
        PlayTime->setObjectName(QStringLiteral("PlayTime"));
        PlayTime->setGeometry(QRect(360, 320, 48, 16));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(PlayTime->sizePolicy().hasHeightForWidth());
        PlayTime->setSizePolicy(sizePolicy1);
        PlayTime->setStyleSheet(QStringLiteral("background-color: rgb(255, 255, 255);"));
        TotalTime = new QLabel(Widget);
        TotalTime->setObjectName(QStringLiteral("TotalTime"));
        TotalTime->setGeometry(QRect(1, 320, 48, 16));
        sizePolicy1.setHeightForWidth(TotalTime->sizePolicy().hasHeightForWidth());
        TotalTime->setSizePolicy(sizePolicy1);
        TotalTime->setStyleSheet(QStringLiteral("background-color: rgb(255, 255, 255);"));
        AudioSlider = new XSlider(Widget);
        AudioSlider->setObjectName(QStringLiteral("AudioSlider"));
        AudioSlider->setGeometry(QRect(410, 320, 41, 22));
        sizePolicy.setHeightForWidth(AudioSlider->sizePolicy().hasHeightForWidth());
        AudioSlider->setSizePolicy(sizePolicy);
        AudioSlider->setMaximum(999);
        AudioSlider->setPageStep(100);
        AudioSlider->setOrientation(Qt::Horizontal);

        retranslateUi(Widget);
        QObject::connect(AudioSlider, SIGNAL(sliderPressed()), Widget, SLOT(AudioSlider_pressed()));
        QObject::connect(AudioSlider, SIGNAL(sliderReleased()), Widget, SLOT(AudioSlider_released()));

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "ColorPlayer", 0));
        PlayTime->setText(QApplication::translate("Widget", "00:00:00", 0));
        TotalTime->setText(QApplication::translate("Widget", "00:00:00", 0));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
