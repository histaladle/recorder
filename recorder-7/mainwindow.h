#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mediarecoder.h"
#include "capturethread.h"
#include "samplethread.h"
#include <QTime>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    MediaRecoder *recorder=NULL;
    CaptureThread *capThrd=NULL;
    SampleThread *samThrd=NULL;
    QTimer recTimer;
    QTime recTime;
signals:
    void recorderStarted();
    void recordStopped();
    void fileSelected(char *file);
    void videoSet(int w,int h,int fps);
    void audioSet(int samplerate,int ch);
private slots:
    void onRecorderStarted();
    void onRecorderStopped();
    void onRecorderTimeUpdated();
private slots:
    void on_startBtn_clicked();

    void on_stopBtn_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
