#ifndef MOUSEHOOK_H
#define MOUSEHOOK_H

#include <QObject>
#include <QDir>
#include <QTimer>
#include <QTime>

#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

class MouseHook : public QObject
{
    Q_OBJECT
public:
    static MouseHook &instance();
    static LRESULT CALLBACK getMouse(int Code, WPARAM wParam, LPARAM lParam);
    void setParameters(int buttons, int timerSeconds);

    bool getLMB() const;
    bool getRMB() const;
    bool getMMB() const;
    bool getMWH() const;

signals:
    void mouseClicked();
    void screenSaved(QString path);

public slots:
    void makeScreenshot();

private:
    HHOOK mHook;
    bool LMB;
    bool RMB;
    bool MMB;
    bool MWH;
    QTimer* timer;
    QString* path;
    QString* pathForGDI;

    MouseHook(QObject *parent = nullptr);
    ~MouseHook() {}
};

#endif // MOUSEHOOK_H
