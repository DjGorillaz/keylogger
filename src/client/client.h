#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTimer>

#include "config.h"
#include "fileserver.h"
#include "fileclient.h"
#include "mousehookWinApi.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0, const QString& path = QDir::currentPath());
    ~Client();

signals:

private slots:
    bool getOnline();
    void getNewFile(const QString& path, const QString& ip);

private:
    void update();
    void getNewConfig(const QString& path);

    QString path;
    QString name;
    QTimer* onlineTimer;
    QTimer* screenTimer;
    Config* config;
    FileServer* fileServer;
    FileClient* fileClient;
};

#endif // CLIENT_H
