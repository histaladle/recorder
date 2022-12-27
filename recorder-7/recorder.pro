#-------------------------------------------------
#
# Project created by QtCreator 2019-08-18T19:21:09
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = recorder
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mediarecoder.cpp \
    audioencoder.cpp \
    mediaencoder.cpp \
    videoencoder.cpp \
    capturethread.cpp \
    samplethread.cpp

HEADERS  += mainwindow.h \
    mediarecoder.h \
    audioencoder.h \
    mediaencoder.h \
    videoencoder.h \
    capturethread.h \
    samplethread.h
INCLUDEPATH += ./../../ffmpeg-4.1.4-win32-dev/include

LIBS += ./../../ffmpeg-4.1.4-win32-shared/bin/avformat-58.dll \
        ./../../ffmpeg-4.1.4-win32-shared/bin/avcodec-58.dll \
        ./../../ffmpeg-4.1.4-win32-shared/bin/avutil-56.dll \
        ./../../ffmpeg-4.1.4-win32-shared/bin/swresample-3.dll \
        ./../../ffmpeg-4.1.4-win32-shared/bin/swscale-5.dll

FORMS    += mainwindow.ui
