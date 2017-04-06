#include "server.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Server* server1;
    QObject::connect(&a, &QCoreApplication::aboutToQuit, [=](){ delete server1; } );

    if (argc <= 1)
    {
        server1 = new Server;
    }
    else if (argc == 3)
    {
        //Set ip + port
        server1 = new Server(0, QDir::currentPath(), QString(argv[1]).toInt() );
    }
    else if (argc > 3)
    {
        //Set path + ip + port
        server1 = new Server(0, argv[1], QString(argv[2]).toInt());
    }

    server1->show();

    return a.exec();
}
