#include "server.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server* server;

    if (argc <= 1)
    {
        server = new Server;
    }
    else if (argc == 3)
    {
        //Set ip + port
        server = new Server(0, QDir::currentPath(), QString(argv[1]).toInt() );
    }
    else if (argc > 3)
    {
        //Set path + ip + port
        server = new Server(0, argv[1], QString(argv[2]).toInt());
    }

    QObject::connect(&a, &QCoreApplication::aboutToQuit, [=](){ server->deleteLater(); } );
    server->show();

    return a.exec();
}
