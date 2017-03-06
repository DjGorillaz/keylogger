#include <QCoreApplication>
#include <QDebug>
#include "client.h"


Client::Client()
{
    //Start modules
    fileServer = new FileServer(0, 1234);
    fileClient = new FileClient(0, "127.0.0.1", 12345);
    config = new Config;
    //add mousehook
    //bind mousehook

    fileServer->start();

    //Load config
    //Default path ./config.cfg
    if( ! loadConfig(*config) )
    {
        qDebug() << "Config not found. Creating new.";
        initDefaultConfig(*config);
        saveConfig(*config);
    }

    update(); //??

    //Get username
    name = qgetenv("USER");
    if (name.isEmpty())
    {
        name = qgetenv("USERNAME");
        if (name.isEmpty())
            name = qgetenv("COMPUTERNAME");
    }

    onlineTimer = new QTimer;

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

}

void Client::update()
{
    qDebug() << "seconds:\t" << config->seconds;
    int buttons = config->mouseButtons;

    //qDebug() << buttons; //((buttons >> 4) & 0x1);

    // 0xLMB_RMB_MMB_MWH
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
    //If cfg received
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
