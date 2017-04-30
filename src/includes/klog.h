#ifndef KLOG_H
#define KLOG_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QDateTime>
#include <QDebug>

#include <windows.h>
#include <Psapi.h>

using namespace std;

class Klog : public QObject
{
    Q_OBJECT
public:
    static Klog &instance();
    static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    void setParameters(const bool& isWorking, const int& timerSeconds);

signals:
    void timerIsUp();

private:
    QTimer* timer;
    QFile* logFile;
    bool isWorking;
    QString currProcess;

    Klog(QObject *parent = nullptr);
    ~Klog() {}
};

#endif // KLOG_H
