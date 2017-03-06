#include "screenshot.h"

Screenshot::Screenshot()
{
    timer = new QTimer();
    path = new QString(QDir::currentPath());
    connect(timer, SIGNAL(timeout()), this, SLOT(makeScreenshot()));
}

Screenshot::Screenshot(QString& defaultPath)
{
    timer = new QTimer();
    path = new QString(defaultPath);
    connect(timer, SIGNAL(timeout()), this, SLOT(makeScreenshot()));
}

Screenshot::~Screenshot()
{
    delete timer;
    delete path;
}

void Screenshot::changeTimer(int ms) //Секунд
{
    if (ms == 0)
        timer->stop();
    else
        timer->start(ms*1000);
}

void Screenshot::makeScreenshot()
{
    QString name = QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy");
    QPixmap originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
    QString savePath (*path + '/' + name + ".png");
    originalPixmap.save(savePath);
    //qDebug() << savePath;

    emit screenshotMade(savePath);
}

int Screenshot::getInterval()
{
    return timer->interval();
}


