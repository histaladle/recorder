#include "mediaencoder.h"
#include <QDebug>
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct SwsContext;
struct AVFrame;
struct AVPacket;
MediaEncoder::MediaEncoder()
{
    if(!avInited)
    {
        av_register_all();
        avcodec_register_all();
        avInited=true;
    }
}

bool MediaEncoder
::open(const char *file)
{
    fileName=new char[strlen(file)];
    memcpy(fileName,file,strlen(file));

    avformat_alloc_output_context2(&oc,NULL,NULL,file);
    if(!oc)
    {
        qDebug() << "oc alloc failed";
        return false;
    }
    return true;
}

bool MediaEncoder::writeHead()
{
    if(!oc)
    {
        return false;
    }
    int re;
    re=avio_open(&oc->pb,fileName,AVIO_FLAG_WRITE);
    if(re<0)
    {
        qDebug() << "avio open failed";
        return false;
    }
    re=avformat_write_header(oc,NULL);
    if(re<0)
    {
        qDebug() << "write head failed";
        return false;
    }
    return true;
}

bool MediaEncoder::writeTrailer()
{
    if(!oc||!oc->pb)
    {
        return false;
    }
    int re;
    re=av_write_trailer(oc);
    if(re)
    {
        qDebug() << "write trailer failed";
        return false;
    }
    re=avio_close(oc->pb);
    if(re)
    {
        qDebug() << "io close failed";
        return false;
    }
    return true;
}

bool MediaEncoder::write(AVPacket *pkt)
{
    if(!pkt||!oc||pkt->size<=0)
    {
        return false;
    }
    int re;

    re=av_interleaved_write_frame(oc,pkt);
    if(re)
    {
        return false;
    }
    return true;
}

void MediaEncoder::close()
{
    if(oc)
    {
        avformat_close_input(&oc);
    }
    if(fileName)
    {
        delete[] fileName;
    }
}

AVFormatContext* MediaEncoder::context()
{
    return oc;
}
