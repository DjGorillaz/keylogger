#include <QCoreApplication>
#include <QDebug>
#include "client.h"

Client::Client(QObject* parent, const QString& defaultPath):
    QObject(parent),
    path(defaultPath)
{
    //Start modules
    fileServer = new FileServer(0, 1234, path);
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
    //TODO: new variable isOnline?

    //Wait for new config file
    connect(fileServer, &FileServer::dataSaved, [this](QString str, QString ip){ this->getNewFile(str, ip); });

    //Connect screenshot module
    connect(&MouseHook::instance(), &MouseHook::mouseClicked, &MouseHook::instance(), &MouseHook::makeScreenshot);
    connect(&MouseHook::instance(), &MouseHook::screenSaved,
            this, [this](QString path){
                                            if (fileClient->connect())
                                            {
                                                fileClient->sendFile(path);
                                                fileClient->disconnect();
                                            }
                                        });
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
    qDebug() << "\nCONFIG:";
    qDebug() << "seconds:\t" << config->seconds;

    // 0 x LMB_RMB_MMB_MWH
    int buttons = config->mouseButtons;

    qDebug() << "LMB:\t" << ((buttons & 0x0008) ? 1 : 0);
    qDebug() << "RMB:\t" << ((buttons & 0x0004) ? 1 : 0);
    qDebug() << "MMB:\t" << ((buttons & 0x0002) ? 1 : 0);
    qDebug() << "MWH:\t" << ((buttons & 0x0001) ? 1 : 0);

    //Update screenshot parameters
    MouseHook::instance().setParameters(buttons, config->seconds);
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
    //Delete old config and rename new
    QFile oldConfig("config.cfg");
    QFile newConfig(path);
    if (oldConfig.exists())
        oldConfig.remove();
    newConfig.rename("config.cfg");

    //Check if folder is empty
    QDir dir = path.section('/', 0, -2);
    if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
        dir.rmdir(dir.dirName());
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Client client1;

    return a.exec();

}
