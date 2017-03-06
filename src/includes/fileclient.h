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
    FileClient(QObject* parent, QString ip, quint16 port);
    ~FileClient();

public slots:
    bool connect();
    void disconnect();
    void changePeer(QString &ip, quint16& port);
    bool sendFile(QString file);
    bool sendStr(QString str);

private:
    QString ip;
    quint16 port;
    QTcpSocket* socket;
};

#endif // FILECLIENT_H
