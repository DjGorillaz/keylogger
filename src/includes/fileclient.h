#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QAbstractSocket>
#include <QTcpSocket>

#include <QDataStream>
#include <QFile>

class FileClient : public QObject
{
    Q_OBJECT
public:
    FileClient(QObject* parent, QString ip, int port);
    ~FileClient();

public slots:
    bool connect();
    bool sendFile(QString file);
    bool sendStr(QString str);

private:
    QString ip;
    int port;
    QTcpSocket* socket;
};

#endif // FILECLIENT_H
