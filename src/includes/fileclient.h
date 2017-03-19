#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include <QList>
#include <QCoreApplication>

class FileClient : public QObject
{
    Q_OBJECT
public:
    FileClient(QObject* parent); //, const QString& ip, const quint16& port);
    ~FileClient();

    bool sendFile(const QString& file);
    bool sendStr(const QString& str);
    void changePeer(const QString &ip, const quint16 &port);
    bool connect();
    void disconnect();
    static QString ip;
    static quint16 port;

signals:
    void disconnected(bool result);

protected:
    QTcpSocket* socket;
};


class SendData : public FileClient
{
public:
    SendData(QObject *parent, QString& path);
    ~SendData();
    bool connectAndSendFile();
    bool connectAndSendString();

private:
    QString path;
};


#endif // FILECLIENT_H
