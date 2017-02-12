#include "fileserver.h"

//Get size of array
qint64 arrToInt(const QByteArray& qba)
{
    qint64 temp;
    QDataStream data(qba);
    data >> temp;
    return temp;
}

//defaultPath = QString(QDir::currentPath())
FileServer::FileServer(QObject* parent, int p, QString defaultPath):
    QObject(parent),
    port(p),
    path(defaultPath)
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

FileServer::~FileServer()
{
    delete server;
}

bool FileServer::start()
{
    return (server->listen(QHostAddress::Any, port));
}

void FileServer::newConnection()
{
    //while (server->hasPedingConnectios())
    QTcpSocket* socket = new QTcpSocket;
    socket = server->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

    QByteArray* buffer = new QByteArray("");
    qint64* qi = new qint64(0);
    QString* fileName = new QString("");
    buffers.insert(socket, buffer);
    sizes.insert(socket, qi);
    names.insert(socket, fileName);
}

/*
 * Recieved packet structure:
 * size(data) + size(file_name) + file_name + data
 *
 * size(data), size(file_name) - qint64
 * file_name, data - QByteArray
 */
void FileServer::readyRead()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    QByteArray* buffer = buffers.value(socket);
    qint64* size = sizes.value(socket);
    QString* fileName = names.value(socket);


    while (socket->bytesAvailable() > 0)
    {
        QByteArray tempArray = socket->readAll();
        buffer->append(tempArray);
        //Read data for the first time
        if (buffer->size() >= 16 && *size == 0)
        {
            *size = arrToInt(buffer->mid(0,8));
            qint64 fileNameSize = arrToInt(buffer->mid(8,8));
            *fileName = QString(buffer->mid(16, fileNameSize));
            //Remove read data
            buffer->remove(0, 16 + fileNameSize);
            tempArray.remove(0, 16+fileNameSize);
        }
        //If we get file
        if (*fileName != "str")
        {
            QFile file(*fileName);
            if(!(file.open(QIODevice::Append)))
            {
                qDebug("File cannot be opened.");
            }
            file.write(tempArray);
            buffer->clear();
            file.close();
            qint64 fileSize = file.size();
            //signal for progress bar
            emit dataGet(fileSize, *size);
            //If receive all data
            if (fileSize >= *size)
            {
                qDebug() << "File received";
                //QString savePath(path + '/' + *(names.value(socket)));
                nullBuffer(socket);
                //emit dataSaved(savePath);
            }
        }
        //If we get string
        else
        {
            //If recieve whole string
            if (buffer->size() >= *size) //(*size + 16 + name->toUtf8().size())
            {
                qDebug() << *(buffers.value(socket));
                emit stringRecieved( *(buffers.value(socket)) );
                nullBuffer(socket);
            }
        }
    }

}

void FileServer::disconnected()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());

    //Delete buffers
    delete buffers.value(socket);
    delete sizes.value(socket);
    delete names.value(socket);

    buffers.remove(socket);
    sizes.remove(socket);
    names.remove(socket);

    qDebug() << "delete socket";
    delete socket;
}

//Null buffer before receiving next data
void FileServer::nullBuffer(QTcpSocket* socket)
{
    *(sizes.value((socket))) = 0;
    buffers.value(socket)->clear();
    names.value(socket)->clear();
}

void FileServer::progress(qint64 current, qint64 overall)
{
    qDebug() << current/1024/1024 << "MB of " << overall/1024/1024 << "MB";
}


