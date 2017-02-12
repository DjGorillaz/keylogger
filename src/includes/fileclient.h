#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QCoreApplication>

#include <QAbstractSocket>
#include <QTcpSocket>

#include <QDataStream>
#include <QFile>


enum TYPE { _FILE, _STRING };


class FileClient : public QObject
{
    Q_OBJECT
public:
    FileClient(QObject* parent = 0, QString ip = 0, int port = 0);
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
