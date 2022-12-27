#ifndef MEDIAENCODER_H
#define MEDIAENCODER_H

#include <QDebug>
struct AVCodecContext;
struct AVFormatContext;
struct AVStream;
struct SwrContext;
struct AVFrame;
struct AVPacket;
class MediaEncoder
{
public:
    MediaEncoder();
public:
    bool open(const char *file);
    void close();
    bool writeHead();
    bool write(AVPacket *pkt);
    bool writeTrailer();
    AVFormatContext* context();
protected:
    AVFormatContext *oc=NULL;
    bool avInited=false;
    char *fileName=NULL;
    long long frmIndex=0;
};

#endif // MEDIAENCODER_H
