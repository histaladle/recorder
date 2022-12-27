#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H

#include <QDebug>
struct AVCodecContext;
struct AVFormatContext;
struct AVStream;
struct SwrContext;
struct AVFrame;
struct AVPacket;
class AudioEncoder
{
public:
    AudioEncoder();
public:
    bool open(AVFormatContext *oc,
              int samplerate,int channels,
              int sample_fmt,int bitrate=64000);
    bool enqueue(unsigned char *pcm);
    bool enqueue(unsigned char *pcm,long long ms);
    AVPacket* dequeue();
    bool flush();
    void close();
    int frameSize();
    double timebase();
protected:
    AVCodecContext *aedctx=NULL;
    AVStream *as=NULL;
    SwrContext *edrctx=NULL;
    AVFrame *tmpAFrm=NULL;
    long long frmIndex=0;
    long long inms=0;
};

#endif // AUDIOENCODER_H
