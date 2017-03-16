#include "fileserver.h"

//Get size of array
qint64 arrToInt(const QByteArray& qba)
{
    qint64 temp;
    QDataStream data(qba);
    data >> temp;
    return temp;
}

//Get peer name or ip
QString getName(const QTcpSocket* socket)
{
    QString name = socket->peerName();
    if( name.isNull() )
    {
        quint32 ipv4 = socket->peerAddress().toIPv4Address();
        name =  QString::number( (ipv4 >> 24) & 0xFF ) + '.' +
                QString::number( (ipv4 >> 16) & 0xFF ) + '.' +
                QString::number( (ipv4 >> 8) & 0xFF ) + '.' +
                QString::number( ipv4 & 0xFF );
    }
    return name;
}

//defaultPath = QString(QDir::currentPath())
FileServer::FileServer(QObject* parent, int p, QString defaultPath):
    QObject(parent),
    port(p),
    path(defaultPath)
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &FileServer::newConnection);
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
    QTcpSocket* socket =  server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &FileServer::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &FileServer::disconnected);

    QByteArray* buffer = new QByteArray("");
    qint64* qi = new qint64(0);
    QString* fileName = new QString("");
    buffers.insert(socket, buffer);
    sizes.insert(socket, qi);
    names.insert(socket, fileName);
    areNamesFinal.insert(socket, false);

    //Make subfolder for each user
    QString subFolder = getName(socket);
    QDir dir;
    dir.mkpath(path + "/" + subFolder);
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

    //For differrent users
    QString subFolder = getName(socket);

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
        }
        //If we get file
        if (*fileName != "str")
        {
            QFile file(path + '/' + subFolder + '/' + *fileName);
            QString newFileName;
            int ctr = 1;
            //If file already exists add (i)
            if (file.exists() && areNamesFinal.value(socket) == false)
            {
                while (file.exists())
                {
                    newFileName = fileName->section('.', 0, -2) +       //file name
                                " (" + QString::number(ctr) + ")." +    //(i).
                                fileName->section('.', -1, -1);         //extension
                    file.setFileName(newFileName);
                     ++ctr;
                }
                //rename file in QHash
                *fileName =  newFileName;
            }
            if (areNamesFinal.value(socket)== false)
                areNamesFinal[socket] = true;

            //Open file and write to it
            if(!(file.open(QIODevice::Append)))
            {
                qDebug("File cannot be opened.");
            }
            qint64 fileSize = file.size();

            //Signal for progress bar
            emit dataGet(fileSize, *size);

            if (fileSize + buffer->size() < *size)
            {
                file.write(*buffer); //tempArray
                buffer->clear();
                file.close();
            }
            //If we receive all data and
            //buffer size + file size >= actual file size
            else
            {
                //Write to file first (*size - fileSize) bytes from buffer
                file.write(buffer->left(*size - fileSize));
                buffer->remove(0, *size - fileSize);
                file.close();
                qDebug() << "File received";

                QString savePath(path + '/' + subFolder + '/' + *(names.value(socket)));
                nullBuffer(socket);
                emit dataSaved(savePath, subFolder);
            }
        }
        //If we get string
        else
        {
            //If recieve whole string
            if (buffer->size() >= *size) //(*size + 16 + name->toUtf8().size())
            {
                qDebug() << *(buffers.value(socket));
                emit stringRecieved( *(buffers.value(socket)), subFolder );
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
    areNamesFinal.remove(socket);

    qDebug() << "Client disconnected";
    socket->deleteLater();
}

//Null buffer before receiving next data
void FileServer::nullBuffer(QTcpSocket* socket)
{
    *(sizes.value((socket))) = 0;
    //buffers.value(socket)->clear();
    names.value(socket)->clear();
    areNamesFinal[socket] = false;
}

void FileServer::progress(qint64 current, qint64 overall)
{
    qDebug() << current/1024/1024 << "MB of " << overall/1024/1024 << "MB";
}


