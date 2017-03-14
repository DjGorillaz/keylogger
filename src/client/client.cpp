#include <QCoreApplication>
#include <QDebug>
#include "client.h"


Client::Client()
{
    //Start modules
    fileServer = new FileServer(0, 1234);
    fileClient = new FileClient(0, "127.0.0.1", 12345);
    config = new Config;
    onlineTimer = new QTimer;
    screenTimer = new QTimer;

    fileServer->start();

    //TODO: add mousehook
    //bind mousehook

    //Load config
    //Default path ./config.cfg
    if( ! loadConfig(*config) )
    {
        qDebug() << "Config not found. Creating new.";
        saveConfig(*config);
    }

    //Update client state
    update();

    //Get username
    name = qgetenv("USER");
    if (name.isEmpty())
    {
        name = qgetenv("USERNAME");
        if (name.isEmpty())
            name = qgetenv("COMPUTERNAME");
    }

    //Trying to connect to server
    if ( !getOnline() )
    {
        connect(onlineTimer, &QTimer::timeout, this, &Client::getOnline);
        onlineTimer->start(30*1000);    //30 sec
    }
    //new variable isOnline?

    //Wait for new config file
    connect(fileServer, &FileServer::dataSaved, [this](QString str, QString ip){ this->getNewFile(str, ip); });
}

Client::~Client()
{
    fileClient->sendStr("OFFLINE:" + name);
    delete onlineTimer;
    delete screenTimer;
    delete config;
    delete fileServer;
    delete fileClient;
}

void Client::update()
{
    qDebug() << "seconds:\t" << config->seconds;
    int buttons = config->mouseButtons;

    //qDebug() << buttons; //((buttons >> 4) & 0x1);

    // 0 x LMB_RMB_MMB_MWH
    qDebug() << "LMB:\t" << ((buttons >> 3 & 0x1) ? 1 : 0);
    qDebug() << "RMB:\t" << ((buttons >> 2 & 0x1) ? 1 : 0);
    qDebug() << "MMB:\t" << ((buttons >> 1 & 0x1) ? 1 : 0);
    qDebug() << "MWH:\t" << ((buttons & 0x1) ? 1 : 0);
}

bool Client::getOnline()
{
    if (fileClient->connect() && fileClient->sendStr("ONLINE:" + name))
    {
        onlineTimer->stop();
        fileClient->disconnect();
        return true;
    }
    else
        return false;
}

void Client::getNewFile(const QString& path, const QString & /*ip*/)
{
    //TODO: if not online => send str online
    QString extension = path.section('.', -1, -1);
    //If config received
    if (extension == "cfg")
        getNewConfig(path);
}

void Client::getNewConfig(const QString &path)
{
    loadConfig(*config, path);
    update();
    //if config file already exists or config has another name
    if (path.section('/', -1, -1) != "config.cfg")
    {
        QFile oldConfig("config.cfg");
        QFile newConfig(path);
        //Remove old config and rename new
        if (oldConfig.exists())
            oldConfig.remove();
        newConfig.rename("config.cfg");
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Client client1;

    return a.exec();

}
