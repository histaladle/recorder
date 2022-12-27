#include "audioencoder.h"
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
AudioEncoder::AudioEncoder()
{

}

bool AudioEncoder
::open(AVFormatContext *oc,
       int samplerate, int channels,
       int sample_fmt, int bitrate)
{
    if(!oc)
    {
        return false;
    }
    int re;
    AVCodec *aed=avcodec_find_encoder(AV_CODEC_ID_AAC);
    if(!aed)
    {
        qDebug() << "encoder fined failed";
        return false;
    }
    aedctx=avcodec_alloc_context3(aed);
    if(!aedctx)
    {
        qDebug() << "encoder context alloc failed";
    }
    aedctx->sample_rate=samplerate;
    aedctx->bit_rate=bitrate;
    aedctx->sample_fmt=(AVSampleFormat)sample_fmt;
    aedctx->channel_layout=av_get_default_channel_layout(channels);
    aedctx->channels=channels;
    aedctx->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    re=avcodec_open2(aedctx,aed,NULL);
    if(re)
    {
        qDebug() << "encoder context open failed";
        avcodec_free_context(&aedctx);
        return false;
    }
    as=avformat_new_stream(oc,aed);
    as->codecpar->codec_tag=0;
    avcodec_parameters_from_context(as->codecpar,aedctx);

    edrctx=swr_alloc_set_opts(edrctx,
                              aedctx->channel_layout,
                              aedctx->sample_fmt,
                              aedctx->sample_rate,
                              aedctx->channel_layout,
                              AV_SAMPLE_FMT_S16,
                              aedctx->sample_rate,
                              0,NULL);
    if(!edrctx)
    {
        return false;
    }
    re=swr_init(edrctx);
    if(re)
    {
        qDebug() << "swr init error";
        return false;
    }

    tmpAFrm=av_frame_alloc();
    tmpAFrm->format=sample_fmt;
    tmpAFrm->nb_samples=1024;
    tmpAFrm->sample_rate=samplerate;
    tmpAFrm->channels=channels;
    tmpAFrm->channel_layout=av_get_default_channel_layout(channels);
    re=av_frame_get_buffer(tmpAFrm,0);
    if(re<0)
    {
        qDebug() << "frame get buffer failed";
        return false;
    }
    return true;
}

bool AudioEncoder::enqueue(unsigned char *pcm)
{
    if(!pcm||!aedctx||!edrctx)
    {
        return false;
    }
    int re;
    const uint8_t *idata[AV_NUM_DATA_POINTERS]={0};
    idata[0]=pcm;
    int rl;
    rl=swr_convert(edrctx,tmpAFrm->data,
                   tmpAFrm->nb_samples,
                   idata,tmpAFrm->nb_samples);
    if(rl<=0)
    {
        qDebug() << "swr convert failed";
        return false;
    }
    tmpAFrm->pts=frmIndex*tmpAFrm->nb_samples;
    re=avcodec_send_frame(aedctx,tmpAFrm);
    if(re<0)
    {
        qDebug() << "send frame failed";
        return false;
    }
    frmIndex++;
    return true;
}

bool AudioEncoder::enqueue(unsigned char *pcm, long long ms)
{
    if(ms<=inms)
    {
        return false;
    }
    if(!pcm||!aedctx||!edrctx)
    {
        return false;
    }
    int re;
    const uint8_t *idata[AV_NUM_DATA_POINTERS]={0};
    idata[0]=pcm;
    int rl;
    rl=swr_convert(edrctx,tmpAFrm->data,
                   tmpAFrm->nb_samples,
                   idata,tmpAFrm->nb_samples);
    if(rl<=0)
    {
        qDebug() << "swr convert failed";
        return false;
    }
//    tmpAFrm->pts=ms*aedctx->time_base.den
//            /(aedctx->time_base.num*1000);
//    qDebug() << "debug1" << tmpAFrm->nb_samples;
    tmpAFrm->pts=ms*tmpAFrm->sample_rate/1000;
    re=avcodec_send_frame(aedctx,tmpAFrm);
    if(re<0)
    {
        qDebug() << "send frame failed";
        return false;
    }
    inms=ms;
    frmIndex++;
    return true;
}

AVPacket* AudioEncoder::dequeue()
{
    if(!aedctx||!edrctx)
    {
        return NULL;
    }
    int re;
    AVPacket *pkt;
    pkt=av_packet_alloc();
    re=avcodec_receive_packet(aedctx,pkt);
    if(re)
    {
        av_packet_free(&pkt);
        return NULL;
    }
    pkt->stream_index=as->index;
    av_packet_rescale_ts(pkt,aedctx->time_base,as->time_base);
    pkt->dts=pkt->pts;
//    qDebug() << "a stream index" << as->index << pkt->dts
//             << pkt->pts;
    return pkt;
}

void AudioEncoder::close()
{
    if(aedctx)
    {
        avcodec_close(aedctx);
        avcodec_free_context(&aedctx);
    }
    if(edrctx)
    {
        swr_free(&edrctx);
    }
    if(tmpAFrm)
    {
        av_frame_free(&tmpAFrm);
    }
}

bool AudioEncoder::flush()
{
    int re;
    re=avcodec_send_frame(aedctx,NULL);
    if(re<0)
    {
        return false;
    }
    return true;
}

int AudioEncoder::frameSize()
{
    if(tmpAFrm)
    {
        return tmpAFrm->nb_samples*tmpAFrm->channels*2;
    }
    return 0;
}

double AudioEncoder::timebase()
{
    double tb=0.0;
    if(!as)
    {
        qDebug() << "no stream";
        return 0.0;
    }
    tb=as->time_base.num*1.0/as->time_base.den;
    return tb;
}
