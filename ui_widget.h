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
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include "videowidget.h"
#include "xslider.h"

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    VideoWidget *openGLWidget;
    QPushButton *OpenButton;
    QPushButton *PlayButton;
    XSlider *PlaySlider;
    QLabel *PlayTime;
    QLabel *SP;
    QLabel *TotalTime;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(459, 352);
        openGLWidget = new VideoWidget(Widget);
        openGLWidget->setObjectName(QStringLiteral("openGLWidget"));
        openGLWidget->setGeometry(QRect(0, 0, 461, 271));
        openGLWidget->setMaximumSize(QSize(1920, 1080));
        OpenButton = new QPushButton(Widget);
        OpenButton->setObjectName(QStringLiteral("OpenButton"));
        OpenButton->setGeometry(QRect(4, 326, 47, 23));
        OpenButton->setStyleSheet(QStringLiteral(""));
        PlayButton = new QPushButton(Widget);
        PlayButton->setObjectName(QStringLiteral("PlayButton"));
        PlayButton->setGeometry(QRect(57, 326, 46, 23));
        PlayButton->setStyleSheet(QStringLiteral(""));
        PlaySlider = new XSlider(Widget);
        PlaySlider->setObjectName(QStringLiteral("PlaySlider"));
        PlaySlider->setGeometry(QRect(0, 280, 461, 19));
        PlaySlider->setMaximum(999);
        PlaySlider->setPageStep(100);
        PlaySlider->setOrientation(Qt::Horizontal);
        PlayTime = new QLabel(Widget);
        PlayTime->setObjectName(QStringLiteral("PlayTime"));
        PlayTime->setGeometry(QRect(66, 300, 48, 16));
        PlayTime->setStyleSheet(QStringLiteral("background-color: rgb(255, 255, 255);"));
        SP = new QLabel(Widget);
        SP->setObjectName(QStringLiteral("SP"));
        SP->setGeometry(QRect(54, 300, 16, 16));
        SP->setStyleSheet(QStringLiteral("border-color: rgb(85, 255, 127);"));
        TotalTime = new QLabel(Widget);
        TotalTime->setObjectName(QStringLiteral("TotalTime"));
        TotalTime->setGeometry(QRect(0, 300, 48, 16));
        TotalTime->setStyleSheet(QStringLiteral("background-color: rgb(255, 255, 255);"));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "ColorPlayer", 0));
        OpenButton->setText(QApplication::translate("Widget", "File", 0));
        PlayButton->setText(QApplication::translate("Widget", "Play", 0));
        PlayTime->setText(QApplication::translate("Widget", "00:00:00", 0));
        SP->setText(QApplication::translate("Widget", "/", 0));
        TotalTime->setText(QApplication::translate("Widget", "00:00:00", 0));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
