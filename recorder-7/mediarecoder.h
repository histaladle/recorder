#ifndef MEDIARECODER_H
#define MEDIARECODER_H

#include <QObject>
#include "mediaencoder.h"
#include "audioencoder.h"
#include "videoencoder.h"
#include <QThread>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QImage>
#include <QElapsedTimer>
class MediaRecoder : public QObject
{
    Q_OBJECT
public:
    explicit MediaRecoder(QObject *parent = 0);
private:
    struct ImageFrame
    {
        long long ms;
        QImage img;
    };
    struct PcmFrame
    {
        long long ms;
        QByteArray pcm;
    };

    QThread *thread=NULL;
    QTimer *watcher=NULL;
    QElapsedTimer recTimer;
    long long recms=0;
//    QList<QImage> imgs;
//    QList<QByteArray> pcms;
    QList<ImageFrame> imgframes;
    QList<PcmFrame> pcmframes;
    QList<AVPacket*> apkts;
    QList<AVPacket*> vpkts;
protected:
    MediaEncoder encoder;
    VideoEncoder vencoder;
    AudioEncoder aencoder;
    bool edInited=false;
    bool vdInited=false;
    bool adInited=false;
    bool running=false;
signals:
    void inited();
    void videoSet(int w,int h,int fps);
    void audioSet(int samplerate,int ch,int packByte);
    void started();
    void stopped();
public slots:
    void onInited();
    void loop();
    void onFileSelected(char *file);
    void onVideoSet(int w,int h,int fps);
    void onAudioSet(int samplerate, int ch);

    void onAudioRead(long long ms,QByteArray pcm);
    void onImageRead(long long ms,QImage img);

    void start();
    void stop();
};

#endif // MEDIARECODER_H
