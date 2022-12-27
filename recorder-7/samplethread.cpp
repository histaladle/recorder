#include "samplethread.h"
#include <QDebug>
#include <QTime>
SampleThread::SampleThread(QObject *parent) : QObject(parent)
{
    thread=new QThread(this);
    moveToThread(thread);
    thread->start();
    connect(this,SIGNAL(inited()),this,SLOT(onInited()));
    emit inited();
}

void SampleThread::onInited()
{
    watcher=new QTimer(this);
    watcher->setInterval(1);
    watcher->setSingleShot(true);
    connect(watcher,SIGNAL(timeout()),this,SLOT(loop()));
}

void SampleThread::loop()
{
    if(!ain)
    {
        return;
    }
    recms+=sampleTimer.restart();
    QByteArray pcm;
    while(pcm.size()<packByte)
    {
        pcm.append(io->read(packByte-pcm.size()));
    }
    emit sampled(recms,pcm);
    watcher->start();
}

void SampleThread::onAudioSet(int samplerate, int ch, int packByte)
{
    afmt.setSampleRate(samplerate);
    afmt.setChannelCount(ch);
    afmt.setSampleSize(16);
    afmt.setSampleType(QAudioFormat::UnSignedInt);
    afmt.setByteOrder(QAudioFormat::LittleEndian);
    afmt.setCodec("audio/pcm");
    if(!afmt.isValid())
    {
        qDebug() << "audio format error";
        return;
    }
    if(ain)
    {
        ain->deleteLater();
    }
    ain=new QAudioInput(afmt);
    this->packByte=packByte;
}

void SampleThread::start()
{
    io=ain->start();
    if(!io)
    {
        qDebug() << "audio start failed";
        return;
    }
    watcher->start();
    emit started();
    recms=0;
    sampleTimer.start();
}

void SampleThread::stop()
{
    ain->suspend();
    recms+=sampleTimer.elapsed();
}
