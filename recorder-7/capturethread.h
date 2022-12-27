#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <QDesktopWidget>
#include <QThread>
#include <QTime>
class CaptureThread : public QObject
{
    Q_OBJECT
public:
    explicit CaptureThread(QObject *parent = 0);

private:
    QThread *thread=NULL;
    QTimer *watcher=NULL;
    int width=320,height=240,fps=25;
    QScreen *scr=NULL;
    int framenum=0;
    QTime capTime;
    QElapsedTimer capTimer;
    long long recms=0;
protected:
    void open(int w,int h,int fps);
signals:
    void inited();
    void screenCaptured(long long ms, QImage img);
public slots:
    void onInited();
    void onVideoSet(int w,int h,int fps);
    void loop();
    void start();
    void stop();
};

#endif // CAPTURETHREAD_H
