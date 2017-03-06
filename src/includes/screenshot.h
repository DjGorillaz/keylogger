#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QCoreApplication>
#include <QTimer>
#include <QTime>

#include <QDir>
#include <QApplication>
#include <QPixmap>
#include <QDesktopWidget>


class Screenshot : public QObject
{
    Q_OBJECT

public:
    Screenshot();
    Screenshot(QString& defaultPath);
    ~Screenshot();

signals:
    void screenshotMade(QString path);

public slots:
    void makeScreenshot();
    void changeTimer(int seconds);
    int getInterval();

private:
    QTimer* timer;
    QString* path;
};

#endif // SCREENSHOT_H
