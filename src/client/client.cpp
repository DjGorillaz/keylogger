#include <QCoreApplication>
#include <QDebug>
#include "client.h"

//Set ip and port
QString FileClient::ip = "127.0.0.1";
quint16 FileClient::port = 12345;

Client::Client(QObject* parent, const QString& defaultPath):
    QObject(parent),
    path(defaultPath)
{
    isOnline = false;
    //Start modules
    fileServer = new FileServer(0, 1234, path);
    fileClient = new FileClient(0);
    //MouseHook::instance();//.getLMB();

    config = new Config;
    onlineTimer = new QTimer;
    screenTimer = new QTimer;

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

    //Get username
    name = qgetenv("USER");
    if (name.isEmpty())
    {
        name = qgetenv("USERNAME");
        if (name.isEmpty())
            name = qgetenv("COMPUTERNAME");
    }

    //connect(onlineTimer, &QTimer::timeout, this, &Client::getOnline);
    //onlineTimer->start(30*1000); //30 sec

    //Trying to connect to server and start timer
    connect(this, &Client::connected, this, &Client::getOnline); //To stop timer
    connect(onlineTimer, &QTimer::timeout, this, &Client::getOnline);
    onlineTimer->start(30*1000); //30 sec
    getOnline();

    //TODO: new variable isOnline?

    //Wait for new config file
    connect(fileServer, &FileServer::dataSaved, [this](QString str, QString ip){ this->getNewFile(str, ip); });

    //Connect screenshot module
    //connect(&MouseHook::instance(), &MouseHook::mouseClicked, &MouseHook::instance(), &MouseHook::makeScreenshot);
    connect(&MouseHook::instance(), &MouseHook::mouseClicked,
            this, [this]()
    {
        //Thread for making screenshots files
        QThread* thread = new QThread;
        MakeScreen* scr = new MakeScreen(0);
        scr->moveToThread(thread);

        connect(thread, &QThread::started, scr, &MakeScreen::makeScreenshot);
        connect(scr, &MakeScreen::screenSaved, thread, &QThread::quit);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(thread, &QThread::finished, scr, &MakeScreen::deleteLater);

        thread->start();

        connect(scr, &MakeScreen::screenSaved,
        this, [this](QString path)
        {
            //Thread for transferring files
            QThread* thread = new QThread;
            SendData* file = new SendData(0, path);
            file->moveToThread(thread);

            connect(thread, &QThread::started, file, &SendData::connectAndSendFile);
            connect(file, &SendData::disconnected, thread, &QThread::quit);
            connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            connect(thread, &QThread::finished, file, &SendData::deleteLater);

            thread->start();
            });
    });
}

Client::~Client()
{
    /*
    QThread* thread = new QThread;
    QString offline = "OFFLINE:" + name;
    SendData* str = new SendData(0, offline);
    file->moveToThread(thread);

    connect(thread, &QThread::started, str, &SendData::connectAndSendFile);
    connect(str, &SendData::disconnected, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, str, &SendData::deleteLater);
    */

    QString offline = "OFFLINE:" + name;
    SendData str(this, offline);
    str.connectAndSendString();
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
    //Stop timer if client connected
    if (isOnline)
    {
        onlineTimer->stop();
        return true;
    }

    //Create thread for sending string
    QThread* thread = new QThread;
    QString online = "ONLINE:" + name;
    SendData* str = new SendData(0, online);
    str->moveToThread(thread);

    connect(thread, &QThread::started, str, &SendData::connectAndSendString);
    connect(str, &SendData::disconnected, thread, &QThread::quit);
    connect(str, &SendData::disconnected,
            [this] (bool result)
    {
        //Set online if string transmitted
        if (result)
        {
            isOnline = true;
            emit connected();
        }
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, str, &SendData::deleteLater);

    thread->start();
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
    qDebug() << path;
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
