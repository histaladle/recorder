#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    recorder=new MediaRecoder;
    capThrd=new CaptureThread;
    samThrd=new SampleThread;

    connect(this,SIGNAL(fileSelected(char*)),
            recorder,SLOT(onFileSelected(char*)));
    connect(this,SIGNAL(videoSet(int,int,int)),
            recorder,SLOT(onVideoSet(int,int,int)));
    connect(this,SIGNAL(audioSet(int,int)),
            recorder,SLOT(onAudioSet(int,int)));
    connect(this,SIGNAL(recordStopped()),
            recorder,SLOT(stop()));
    connect(this,SIGNAL(recorderStarted()),
            recorder,SLOT(start()));

    connect(recorder,SIGNAL(started()),
            capThrd,SLOT(start()));
    connect(recorder,SIGNAL(stopped()),
            capThrd,SLOT(stop()));
    connect(recorder,SIGNAL(videoSet(int,int,int)),
            capThrd,SLOT(onVideoSet(int,int,int)));
    connect(capThrd,SIGNAL(screenCaptured(long long,QImage)),
            recorder,SLOT(onImageRead(long long,QImage)));

    connect(recorder,SIGNAL(started()),
            samThrd,SLOT(start()));
    connect(recorder,SIGNAL(stopped()),
            samThrd,SLOT(stop()));
    connect(recorder,SIGNAL(audioSet(int,int,int)),
            samThrd,SLOT(onAudioSet(int,int,int)));
    connect(samThrd,SIGNAL(sampled(long long,QByteArray)),
            recorder,SLOT(onAudioRead(long long,QByteArray)));

    connect(recorder,SIGNAL(started()),
            this,SLOT(onRecorderStarted()));
    connect(recorder,SIGNAL(stopped()),
            this,SLOT(onRecorderStopped()));

    recTimer.setInterval(1000);
    connect(&recTimer,SIGNAL(timeout()),
            this,SLOT(onRecorderTimeUpdated()));

    recTime.setHMS(0,0,0);

    emit fileSelected("./../../video/test1.mp4");
    emit videoSet(1366,768,20);
//    emit videoSet(1920,1080,20);
    emit audioSet(44100,2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startBtn_clicked()
{
    emit recorderStarted();
}

void MainWindow::on_stopBtn_clicked()
{
    emit recordStopped();
}

void MainWindow::onRecorderTimeUpdated()
{
    recTime=recTime.addSecs(1);
    ui->recTmLbl->setText(recTime.toString());
}

void MainWindow::onRecorderStarted()
{
    recTimer.start();
    recTime.setHMS(0,0,0);
}

void MainWindow::onRecorderStopped()
{
    recTimer.stop();
}
