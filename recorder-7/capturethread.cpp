#include "capturethread.h"
#include <QDebug>
CaptureThread::CaptureThread(QObject *parent) : QObject(parent)
{
    thread=new QThread(this);
    moveToThread(thread);
    thread->start();

    connect(this,SIGNAL(inited()),this,SLOT(onInited()));
    emit inited();
}

void CaptureThread::onInited()
{
    watcher=new QTimer(this);
    watcher->setInterval(1000/fps);
    watcher->setSingleShot(true);
    connect(watcher,SIGNAL(timeout()),this,SLOT(loop()));

    scr=QGuiApplication::primaryScreen();
}

void CaptureThread::loop()
{
    recms+=capTimer.restart();

    capTime.restart();
    QPixmap pix=scr->grabWindow(QApplication::desktop()->winId());
    QImage image(pix.toImage().scaled(width,height)
                 .convertToFormat(QImage::Format_ARGB32));

    emit screenCaptured(recms,image);

    framenum++;

    int capcost=capTime.elapsed();
    if(1000/fps>capcost)
    {
        qDebug() << "debug6";
        watcher->start(1000/fps-capcost);
    }
    else
    {
        qDebug() << "debug7";
        watcher->start(1);
    }
}

void CaptureThread::onVideoSet(int w, int h, int fps)
{
    this->width=w;
    this->height=h;
    this->fps=fps;
    qDebug() << "set cap video" << w << h << fps;
}

void CaptureThread::start()
{
    watcher->start();
    capTime.setHMS(0,0,0);
    framenum=0;
    capTimer.start();
    recms=0;
}

void CaptureThread::stop()
{
    qDebug() << "cap stopped";
    watcher->stop();
    qDebug() << "frame num" << framenum;
    recms+=capTimer.elapsed();
}
