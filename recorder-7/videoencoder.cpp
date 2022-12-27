#include "videoencoder.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

VideoEncoder::VideoEncoder()
{
}

bool VideoEncoder
::open(AVFormatContext *oc, int w,int h,int fps,int bitrate)
{
    if(!oc)
    {
        return false;
    }
    int re;
    AVCodec *ved=NULL;
    ved=avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!ved)
    {
        qDebug() << "find encoder failed";
        return false;
    }

    vedctx=avcodec_alloc_context3(ved);
    if(!vedctx)
    {
        qDebug() << "encoder alloc failed";
        return false;
    }
    vedctx->bit_rate=bitrate;
    vedctx->width=w;
    vedctx->height=h;
    vedctx->time_base={1,fps};
    vedctx->framerate={fps,1};
    vedctx->gop_size=20;
    vedctx->max_b_frames=0;
    vedctx->pix_fmt=AV_PIX_FMT_YUV420P;
    vedctx->codec_id=AV_CODEC_ID_H264;
//    av_opt_set(vedctx->priv_data,"preset","superfast",0);
    vedctx->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    re=avcodec_open2(vedctx,ved,NULL);
    if(re)
    {
        qDebug() << "codec open failed";
        avcodec_free_context(&vedctx);
        return false;
    }
    vs=avformat_new_stream(oc,ved);
    if(!vs)
    {
        qDebug() << "stream create failed";
        return false;
    }
    vs->codecpar->codec_tag=0;
    avcodec_parameters_from_context(vs->codecpar,vedctx);

    edsctx=sws_getCachedContext(edsctx,w,h,AV_PIX_FMT_BGRA,
                                w,h,AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC,NULL,NULL,NULL);
    if(!edsctx)
    {
        qDebug() << "sws context failed";
        return true;
    }

    if(!tmpVFrm)
    {
        tmpVFrm=av_frame_alloc();
        tmpVFrm->format=AV_PIX_FMT_YUV420P;
        tmpVFrm->width=w;
        tmpVFrm->height=h;
        tmpVFrm->pts=0;
        re=av_frame_get_buffer(tmpVFrm,0);
        if(re)
        {
            qDebug() << "frame buffer error";
            return false;
        }
    }
    return true;
}

bool VideoEncoder::enqueue(unsigned char *rgb)
{
    if(!rgb)
    {
        return false;
    }
    int re;
    uint8_t *idata[AV_NUM_DATA_POINTERS]={0};
    idata[0]=rgb;

    int ilsize[AV_NUM_DATA_POINTERS]={0};
    ilsize[0]=vedctx->width*4;
    int rh=0;
    rh=sws_scale(edsctx,idata,ilsize,0,vedctx->height,
                 tmpVFrm->data,tmpVFrm->linesize);
    if(rh<=0)
    {
        return false;
    }

    tmpVFrm->pts=frmIndex;
    re=avcodec_send_frame(vedctx,tmpVFrm);
    if(re)
    {
        qDebug() << "send frame failed";
        return false;
    }
    frmIndex++;
    return true;
}

bool VideoEncoder::enqueue(unsigned char *rgb, long long ms)
{
    if(ms<=inms)
    {
        return false;
    }
    if(!rgb)
    {
        return false;
    }
    int re;
    uint8_t *idata[AV_NUM_DATA_POINTERS]={0};
    idata[0]=rgb;

    int ilsize[AV_NUM_DATA_POINTERS]={0};
    ilsize[0]=vedctx->width*4;
    int rh=0;
    rh=sws_scale(edsctx,idata,ilsize,0,vedctx->height,
                 tmpVFrm->data,tmpVFrm->linesize);
    if(rh<=0)
    {
        return false;
    }

    tmpVFrm->pts=ms*vedctx->time_base.den/(vedctx->time_base.num*1000);
    qDebug() << "write frame" << tmpVFrm->pts;
    re=avcodec_send_frame(vedctx,tmpVFrm);
    if(re)
    {
        qDebug() << "send frame failed";
        return false;
    }
    inms=ms;
    frmIndex++;
    return true;
}

AVPacket *VideoEncoder::dequeue()
{
    int re;
    AVPacket *pkt=NULL;
    pkt=av_packet_alloc();
    re=avcodec_receive_packet(vedctx,pkt);
    if(re||pkt->size<=0)
    {
        av_packet_free(&pkt);
        return NULL;
    }
    qDebug() << "to rescale" << pkt->pts;
    av_packet_rescale_ts(pkt,vedctx->time_base,vs->time_base);
    pkt->stream_index=vs->index;
    qDebug() << "v stream index" << vs->index
             << vedctx->time_base.num << vedctx->time_base.den
             << vs->time_base.num << vedctx->time_base.den;
    return pkt;
}

bool VideoEncoder::flush()
{
    if(!vedctx)
    {
        return false;
    }
    int re;
    re=avcodec_send_frame(vedctx,NULL);
    if(re)
    {
        return false;
    }
    return true;
}

void VideoEncoder::close()
{
    if(vedctx)
    {
        avcodec_close(vedctx);
        avcodec_free_context(&vedctx);
    }
    if(edsctx)
    {
        sws_freeContext(edsctx);
        edsctx=NULL;
    }
    if(tmpVFrm)
    {
        av_frame_free(&tmpVFrm);
    }
}

double VideoEncoder::timebase()
{
    double tb=0.0;
    if(!vs)
    {
        return 0.0;
    }
    tb=vs->time_base.num*1.0/vs->time_base.den;
    return tb;
}
