#-------------------------------------------------
#
# Project created by QtCreator 2017-06-25T12:17:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += multimedia

TARGET = ColorPlayer
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    xslider.cpp \
    ffmpeg.cpp \
    videowidget.cpp \
    audioplay_sdl2.cpp \
    audiodecodethread.cpp \
    videodecodethread.cpp \
    videooutput.cpp \
    colorplayer.cpp \
    demuxthread.cpp \
    messagequeue.cpp \
    common.cpp

HEADERS  += widget.h \
    xslider.h \
    ffmpeg.h \
    videowidget.h \
    audioplay_sdl2.h \
    audiodecodethread.h \
    videodecodethread.h \
    videooutput.h \
    colorplayer.h \
    demuxthread.h \
    messagequeue.h \
    common.h

FORMS    += widget.ui

INCLUDEPATH += G:\QT5.6.2\ffmpeg-3.2.2-win32-dev\include
INCLUDEPATH += G:\SDL2\include
LIBS += G:\QT5.6.2\ffmpeg-3.2.2-win32-dev\lib\avformat.lib
LIBS += G:\QT5.6.2\ffmpeg-3.2.2-win32-dev\lib\avutil.lib
LIBS += G:\QT5.6.2\ffmpeg-3.2.2-win32-dev\lib\avcodec.lib
LIBS += G:\QT5.6.2\ffmpeg-3.2.2-win32-dev\lib\swscale.lib
LIBS += G:\QT5.6.2\ffmpeg-3.2.2-win32-dev\lib\swresample.lib
LIBS += winmm.lib
LIBS += G:\SDL2\lib\SDL2.lib
LIBS += G:\SDL2\lib\SDL2main.lib
LIBS += G:\SDL2\lib\SDL2test.lib
