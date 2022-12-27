#ifndef SAMPLETHREAD_H
#define SAMPLETHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QAudioFormat>
#include <QAudioInput>
class SampleThread : public QObject
{
    Q_OBJECT
public:
    explicit SampleThread(QObject *parent = 0);
private:
    QThread *thread=NULL;
    QTimer *watcher=NULL;
    QAudioFormat afmt;
    QAudioInput *ain=NULL;
    QIODevice *io=NULL;
    int packByte=1024;
    QElapsedTimer sampleTimer;
    long long recms=0;
signals:
    void inited();
    void started();
    void sampled(long long ms,QByteArray pcm);
public slots:
    void onInited();
    void loop();
    void onAudioSet(int samplerate,int ch,int packByte);
    void start();
    void stop();
};

#endif // SAMPLETHREAD_H
