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
    FileClient(QObject* parent, const QString& ip, const quint16& port);
    ~FileClient();

    bool sendFile(const QString& file);
    bool sendStr(const QString& str);
    void changePeer(const QString &ip, const quint16 &port);
    bool connect();
    void disconnect();

private:
    QString ip;
    quint16 port;
    QTcpSocket* socket;
};

#endif // FILECLIENT_H
