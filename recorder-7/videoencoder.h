#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H

#include <QDebug>
#include <QString>
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct SwsContext;
struct AVFrame;
struct AVPacket;
class VideoEncoder
{
public:
    VideoEncoder();
public:
    void close();

    bool open(AVFormatContext *oc, int w,int h,int fps,int bitrate=4000000);
    bool enqueue(unsigned char *rgb);
    bool enqueue(unsigned char *rgb, long long ms);
    AVPacket *dequeue();
    bool flush();
    double timebase();
protected:
    AVCodecContext *vedctx=NULL;
    AVStream *vs=NULL;
    SwsContext *edsctx=NULL;
    AVFrame *tmpVFrm=NULL;
    long long frmIndex=0;
    long long inms=0.0;
};

#endif // VIDEOENCODER_H
