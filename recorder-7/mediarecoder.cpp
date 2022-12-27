#include "mediarecoder.h"
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
#include <QDebug>
MediaRecoder::MediaRecoder(QObject *parent) : QObject(parent)
{
    thread=new QThread(this);
    moveToThread(thread);
    thread->start();

    connect(this,SIGNAL(inited()),this,SLOT(onInited()));

    emit inited();
}

void MediaRecoder::onInited()
{
    watcher=new QTimer(this);
    connect(watcher,SIGNAL(timeout()),this,SLOT(loop()));
    watcher->setInterval(1);
    watcher->setSingleShot(true);
}

void MediaRecoder::loop()
{
    bool matched;
    QByteArray pcm;
    QImage img;
    PcmFrame pcmfrm;
    ImageFrame imgfrm;
    AVPacket *pkt=NULL;
//    double atm,vtm;
//    recms+=recTime.restart();
//    qDebug() << "rec sec" << recsec;

    while(pcmframes.size()>0)
    {
        recms+=recTimer.restart();
        pcmfrm=pcmframes.takeFirst();
        pcm=pcmfrm.pcm;
        matched=aencoder.enqueue((unsigned char*)pcm.data(),pcmfrm.ms);

        if(!matched)
        {
            continue;
        }
        do{
            pkt=aencoder.dequeue();
            if(!pkt)
            {
                break;
            }
            apkts.append(pkt);
        }while(pkt);
    }

    while(imgframes.size()>0)
    {
        recms+=recTimer.restart();
        imgfrm=imgframes.takeFirst();
        img=imgfrm.img;
        qDebug() << "enq v" << recms;
        matched=vencoder.enqueue(img.bits(),imgfrm.ms);
        if(!matched)
        {
            continue;
        }
        do{
            pkt=vencoder.dequeue();
            if(!pkt)
            {
                break;
            }
            qDebug() << "append to vpkts" << pkt->pts;
            vpkts.append(pkt);
        }while(pkt);
    }

    while(apkts.size()>0)
    {
        encoder.write(apkts.takeFirst());
    }
    while(vpkts.size()>0)
    {
        encoder.write(vpkts.takeFirst());
    }

    watcher->start();
}


void MediaRecoder::start()
{
    if(running)
    {
        return;
    }
    AVPacket *pkt=NULL;
    while(apkts.size()>0)
    {
        pkt=apkts.takeFirst();
        delete pkt;
    }
    while(vpkts.size()>0)
    {
        pkt=vpkts.takeFirst();
        delete pkt;
    }
    pcmframes.clear();
    imgframes.clear();
    if(!edInited||!vdInited||!adInited)
    {
        return;
    }
    bool matched;
    watcher->start();
    matched=encoder.writeHead();
    if(!matched)
    {
        return;
    }
    qDebug() << "recorder started"
             << 1/vencoder.timebase();
    emit started();
    recTimer.start();
    recms=0;
    running=true;
}

void MediaRecoder::stop()
{
    if(!running)
    {
        return;
    }
    running=false;
    edInited=false;
    adInited=false;
    vdInited=false;
    watcher->stop();
    qDebug() << "rec time" << recTimer.elapsed();
    emit stopped();
//    double atm,vtm;
    AVPacket *pkt=NULL;
    bool matched;

    matched=aencoder.flush();
    do{
        pkt=aencoder.dequeue();
        if(!pkt)
        {
            break;
        }
        apkts.append(pkt);
    }while(pkt);

    matched=vencoder.flush();
    do{
        pkt=vencoder.dequeue();
        if(!pkt)
        {
            break;
        }
        vpkts.append(pkt);
    }while(pkt);

//    while(vpkts.size()>0 && apkts.size()>0)
//    {
//        atm=apkts[0]->pts*aencoder.timebase();
//        vtm=vpkts[0]->pts*vencoder.timebase();
//        while(apkts.size()>0)
//        {
//            atm=apkts[0]->pts*aencoder.timebase();
//            if(atm>vtm)
//            {
//                break;
//            }
//            pkt=apkts.takeFirst();
//            qDebug() << "write a in stop"
//                     << pkt->pts*aencoder.timebase()
//                     << pkt->pts;
//            encoder.write(pkt);
//        }
//        while(vpkts.size()>0)
//        {
//            vtm=vpkts[0]->pts*vencoder.timebase();
//            if(atm<vtm)
//            {
//                break;
//            }
//            pkt=vpkts.takeFirst();
//            qDebug() << "write v in stop"
//                     << pkt->pts*vencoder.timebase()
//                     << pkt->pts;
//            encoder.write(pkt);
//        }
//    }
    while(apkts.size()>0)
    {
        qDebug() << "rear write a" << apkts[0]->pts*aencoder.timebase();
        encoder.write(apkts.takeFirst());
    }
    while(vpkts.size()>0)
    {
        qDebug() << "rear write v" << vpkts[0]->pts*vencoder.timebase();
        encoder.write(vpkts.takeFirst());
    }
    encoder.writeTrailer();
    qDebug() << "recorder stopped";
}

void MediaRecoder::onFileSelected(char *file)
{
    edInited=encoder.open(file);
}

void MediaRecoder::onVideoSet(int w, int h, int fps)
{
    vdInited=vencoder.open(encoder.context(),w,h,fps);
    emit videoSet(w,h,fps);
}

void MediaRecoder::onAudioSet(int samplerate, int ch)
{
    adInited=aencoder.open(encoder.context(),
                           samplerate,ch,AV_SAMPLE_FMT_FLTP);
    emit audioSet(samplerate,ch,aencoder.frameSize());
}

void MediaRecoder::onAudioRead(long long ms, QByteArray pcm)
{
    if(pcm.isEmpty())
    {
        return;
    }
    qDebug() << "pcm read" << ms;
    PcmFrame pcmfrm;
    pcmfrm.ms=ms;
    pcmfrm.pcm=pcm;
    pcmframes.append(pcmfrm);
}

void MediaRecoder::onImageRead(long long ms, QImage img)
{
    if(img.byteCount()<=0)
    {
        return;
    }
    qDebug() << "img read" << img.size() << ms;
    ImageFrame imgfrm;
    imgfrm.ms=ms;
    imgfrm.img=img;
    imgframes.append(imgfrm);
}
