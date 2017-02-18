#include "fileclient.h"
#include <QThread>

//Convert qint64 to QByteArray
QByteArray intToArr(qint64 value)
{
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << value;
    return temp;
}

FileClient::~FileClient()
{
    if (socket->state() != QAbstractSocket::UnconnectedState)
        socket->close();
    delete socket;
    qDebug() << "socket closed";
}

FileClient::FileClient(QObject* parent, QString i, int p):
    QObject(parent),
    ip(i),
    port(p)
{
    socket = new QTcpSocket(this);
}

bool FileClient::connect()
{
    qDebug() << "Connecting to " << ip << ':' << port;
    socket->connectToHost(ip, port, QIODevice::WriteOnly);
    if(socket->waitForConnected(5000))
    {
        qDebug() << "Connected";
        return true;
    }
    else
    {
        qDebug() << "Error: " << socket->errorString();
        return false;
    }
}

/*
 * Packet structure:
 * size(string) + size("str") + "str" + string
 *
 * size(string), size("str") - qint64
 * string, "str" - QByteArray
 */
bool FileClient::sendStr(QString str)
{
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        //Write size(string)
        QByteArray fileSize = intToArr(str.toUtf8().size());
        socket->write(fileSize);

        //Write size("str") and "str"
        QString fileName = "str";
        QByteArray fileNameArr = fileName.toUtf8();
        socket->write(intToArr(fileNameArr.size()));
        socket->write(fileNameArr);

        //Write string
        socket->write(str.toUtf8());

        if (socket->waitForBytesWritten())
        {
            qDebug() << "Data transmitted";
            return true;
        }
        else
        {
            qDebug() << "Error: " << socket->errorString();
            return false;
        }
    }
    else
    {
        qDebug() << "No conection established";
        return false;
    }
}

/*
 * Packet structure:
 * size(data) + size(file_name) + file_name + data
 *
 * size(data), size(file_name) - qint64
 * file_name, data - QByteArray
 */
bool FileClient::sendFile(QString path)
{
    if(socket->state() == QAbstractSocket::ConnectedState)
    {

        QFile file(path);
        if ( ! file.open(QIODevice::ReadOnly))
        {
            qDebug() << "Couldn't open the file";
            return false;
        }

        //Write size(file)
        QByteArray fileSize = intToArr(file.size());
        socket->write(fileSize);

        //Write size(file_name) + file_name
        QString fileName = file.fileName();
        fileName = fileName.section('/',-1,-1);
        QByteArray fileNameArr = fileName.toUtf8();
        socket->write(intToArr(fileNameArr.size()));
        socket->write(fileNameArr);

        //Write file by chunks
        QByteArray fileArray = file.read(32768*8);
        bool result = false;
        while( !fileArray.isEmpty())
        {
            //qDebug() << "Read : " << fileArray.size();
            socket->write(fileArray);
            result = socket->waitForBytesWritten();
            fileArray.clear();
            fileArray = file.read(32768*8);
        }

        if (result)
        {
            qDebug() << "Data transmitted";
            return true;
        }
        else
        {
            qDebug() << "Error: " << socket->error();
            return false;
        }
        file.close();
    }
    else
    {
        qDebug() << "No conection established";
        return false;
    }

    return false;
}



