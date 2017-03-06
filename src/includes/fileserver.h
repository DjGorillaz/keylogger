#ifndef FILESERVER_H
#define FILESERVER_H

#include <QTcpServer>
#include <QTcpSocket>

#include <QDataStream>
#include <QFile>
#include <QDir>

class FileServer : public QObject
{
    Q_OBJECT
public:
    FileServer(QObject *parent, int port, QString defaultPath = QString(QDir::currentPath()));
    ~FileServer();
    bool start();

signals:
    void dataSaved(QString path, QString ip);
    void stringRecieved(QString string, QString ip);
    void dataGet(qint64, qint64);

private slots:
    void newConnection();
    void readyRead();
    void disconnected();
    void progress(qint64, qint64); //test function

private:
    void nullBuffer(QTcpSocket*);
    int port;
    QString path;
    QTcpServer* server;
    QHash<QTcpSocket*, QByteArray*> buffers;
    QHash<QTcpSocket*, qint64*> sizes;
    QHash<QTcpSocket*, QString*> names;
    QHash<QTcpSocket*, bool> areNamesFinal;
};

#endif // FILESERVER_H
