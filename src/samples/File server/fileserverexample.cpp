#include "fileserver.h"

#include <QCoreApplication>




int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FileServer* server1 = new FileServer(0, 12345);
     //QObject::connect(server1, SIGNAL(dataGet(QTcpSocket*)),  server1, SLOT(testFunc(QTcpSocket*)));
    QObject::connect(server1, SIGNAL(dataGet(qint64,qint64)), server1, SLOT(progress(qint64,qint64)));
    qDebug() << server1->start();

    return a.exec();
}
