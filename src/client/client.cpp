#include <QDebug>
#include "client.h"

Client::Client(QObject* parent, const QString& defaultPath, QString _ip, quint16 _port):
    QObject(parent),
    path(defaultPath),
    ip(_ip),
    port(_port)
{
    //Start modules
    fileServer = new FileServer(this, 1234, path);
    fileClient = new FileClient(this, ip, port);
    config = new Config;
    onlineTimer = new QTimer(this);
    screenTimer = new QTimer(this);

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

    //Trying to connect to server
    getOnline();

    //TODO: new variable isOnline?

    //Wait for new config file
    connect(fileServer, &FileServer::dataSaved, [this](QString str, QString ip){ this->getNewFile(str, ip); });

    //Progress bar
    //connect(fileServer, &FileServer::dataGet, [this](qint64 a, qint64 b){ qDebug() << a/1024/1024 << b/1024/1024; });

    //Connect screenshot module
    /*
    connect(&MouseHook::instance(), &MouseHook::mouseClicked, &MouseHook::instance(), &MouseHook::makeScreenshot);
    connect(&MouseHook::instance(), &MouseHook::screenSaved,
            this, [this](QString path){
                                            if (fileClient->connect())
                                            {
                                                fileClient->sendFile(path);
                                                fileClient->disconnect();
                                            }
                                        });
    */
}

Client::~Client()
{
    fileClient->getOffline();
    delete onlineTimer;
    delete screenTimer;
    delete config;
    delete fileClient;
    delete fileServer;
    qDebug() << "Client deleted.";
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
    //TODO
    //MouseHook::instance().setParameters(buttons, config->seconds);
}

void Client::getOnline()
{
    onlineTimer->start(30*1000);    //30 sec
    connect(onlineTimer, &QTimer::timeout, fileClient, &FileClient::connect);
    connect(fileClient, &FileClient::transmitted, onlineTimer, &QTimer::stop);
    //Send string
    fileClient->enqueueData(_STRING, "ONLINE:" + fileClient->getName());
    fileClient->connect();
}

void Client::getNewFile(const QString& path, const QString & /*ip*/)
{
    qDebug() << path;
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
        dir.rmdir(path.section('/', 0, -2)); //dir.name()
}

