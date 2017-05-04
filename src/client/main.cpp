#include "client.h"
#include <QCoreApplication>

static QCoreApplication* app;

BOOL CtrlHandler( DWORD fdwCtrlType )
{
  switch( fdwCtrlType )
  {
    case CTRL_C_EVENT:
      app->quit();
      return( TRUE );

    case CTRL_CLOSE_EVENT:
      app->quit();
      return( TRUE );

    case CTRL_BREAK_EVENT:
      app->quit();
      return TRUE;

    case CTRL_LOGOFF_EVENT:
      return FALSE;

    case CTRL_SHUTDOWN_EVENT:
      return FALSE;

    default:
      return FALSE;
  }
}

int main(int argc, char *argv[])
{
    app = new QCoreApplication(argc, argv);
    Client* client1;

    //Set console ctrl handler
    if( ! SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE ) )
        qDebug() << "Handle installation error.";

    //Get command line arguments
    if (argc <= 1)
    {
        client1 = new Client(app, QDir::currentPath(), "127.0.0.1", 12345);
    }
    else if (argc == 3)
    {
        //Set ip + port
        client1 = new Client(app, QDir::currentPath(), argv[1], QString(argv[2]).toInt());
    }
    else if (argc > 3)
    {
        //Set path + ip + port
        client1 = new Client(app, argv[1], argv[2], QString(argv[3]).toInt());
    }

    QObject::connect(app, &QCoreApplication::aboutToQuit, client1, &Client::deleteLater);

    return app->exec();
}
