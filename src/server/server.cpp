#include "server.h"
#include "ui_server.h"

//https://www.iconfinder.com/icons/46254/active_approval_online_status_icon#size=16
//https://www.iconfinder.com/icons/46252/busy_offline_status_icon#size=16
//Yusuke Kamiyamane
//License: Creative Commons (Attribution 3.0 Unported)

Server::Server(QWidget *parent, const QString& defaultPath) :
    QMainWindow(parent),
    path(defaultPath),
    ui(new Ui::Server)
{
    ui->setupUi(this);

    //Disable buttons
    ui->buttonLoadConfig->setEnabled(false);
    ui->buttonSaveConfig->setEnabled(false);
    ui->buttonSendConfig->setEnabled(false);
    ui->spinSeconds->setEnabled(false);
    ui->checkLMB->setEnabled(false);
    ui->checkRMB->setEnabled(false);
    ui->checkMMB->setEnabled(false);
    ui->checkMWH->setEnabled(false);

    loadUsers();

    //Setup model
    setupModels();
    ui->treeUsers->setModel(treeModel);

    //Setup mapper
    uiMapper = new QDataWidgetMapper(this);
    uiMapper->setModel(treeModel);
    uiMapper->addMapping(ui->spinSeconds, 2, "value");
    uiMapper->addMapping(ui->checkLMB, 3);
    uiMapper->addMapping(ui->checkRMB, 4);
    uiMapper->addMapping(ui->checkMMB, 5);
    uiMapper->addMapping(ui->checkMWH, 6);

    //Hide model items in tree view
    ui->treeUsers->setColumnHidden(2, true);
    ui->treeUsers->setColumnHidden(3, true);
    ui->treeUsers->setColumnHidden(4, true);
    ui->treeUsers->setColumnHidden(5, true);
    ui->treeUsers->setColumnHidden(6, true);

    connect(ui->treeUsers->selectionModel(), &QItemSelectionModel::currentRowChanged,
            [this](const QModelIndex& current, const QModelIndex&  /* previous */ ) {
                uiMapper->setCurrentIndex(current.row());
                static bool first = true;
                if (first)
                {
                    ui->buttonLoadConfig->setEnabled(true);
                    ui->buttonSaveConfig->setEnabled(true);
                    ui->buttonSendConfig->setEnabled(true);
                    ui->spinSeconds->setEnabled(true);
                    ui->checkLMB->setEnabled(true);
                    ui->checkRMB->setEnabled(true);
                    ui->checkMMB->setEnabled(true);
                    ui->checkMWH->setEnabled(true);
                    first = false;
                }
    });

    //Start modules
    fileServer = new FileServer(this, 12345, path + "/users");
    fileClient = new FileClient(this, "127.0.0.1", 1234);
    fileServer->start();

    QDir configDir;
    configDir.mkpath(path + "/configs");

    connect(fileServer, &FileServer::stringRecieved, [this](QString str, QString ip) { this->getString(str, ip); });

    //Connect buttons clicks
    connect(ui->buttonSendConfig, &QPushButton::clicked, this, &Server::configSendClicked);
    connect(ui->buttonSaveConfig, &QPushButton::clicked, this, &Server::configSaveClicked);
    connect(ui->buttonLoadConfig, &QPushButton::clicked, this, &Server::configLoadClicked);
}

Server::~Server()
{
    saveUsers();
    delete uiMapper;
    delete treeModel;
    delete fileServer;
    delete fileClient;
    delete ui;
}

void Server::setupModels()
{
    treeModel = new QStandardItemModel(this);
    treeModel->setColumnCount(7);

    //Set header names
    treeModel->setHeaderData(0, Qt::Horizontal, "username");
    treeModel->setHeaderData(1, Qt::Horizontal, "ip");
    treeModel->setHeaderData(2, Qt::Horizontal, "seconds");
    treeModel->setHeaderData(3, Qt::Horizontal, "LMB");
    treeModel->setHeaderData(4, Qt::Horizontal, "RMB");
    treeModel->setHeaderData(5, Qt::Horizontal, "MMB");
    treeModel->setHeaderData(6, Qt::Horizontal, "MWH");

    //Traverse existing users and add to model
    QList<QStandardItem*> items;
    QHash<QString, QString>::iterator iter = usernames.begin();
    Config* cfg;
    QString ip;
    QString username;
    while (iter != usernames.end())
    {
        ip = iter.key();
        username = iter.value();

        //load configs
        if (usersConfig.contains(ip))
            cfg = usersConfig.value(ip);
        else
        {
            cfg = new Config();
            usersConfig.insert(ip, cfg);
            if (!loadConfig(*cfg, path + "/configs/" + ip + ".cfg"))
                saveConfig(*cfg, path + "/configs/" + ip + ".cfg");
        }

        initTreeModel(items, ip, username, cfg, OFFLINE);
        ++iter;
    }
}

void Server::initTreeModel(QList<QStandardItem*> &items,
                           const QString &ip,
                           const QString &username,
                           const Config *cfg,
                           const state& st)
{
    //Create items and write to model
    QStandardItem* ipItem = new QStandardItem(ip);
    QStandardItem* nameItem = new QStandardItem(QIcon(st ? ":/icons/online.png" : ":/icons/offline.png"), username);
    //Set not editable item
    ipItem->setFlags(ipItem->flags() & ~Qt::ItemIsEditable);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);

    QStandardItem* secondsItem = new QStandardItem(QString::number(cfg->seconds));
    QStandardItem* LMB = new QStandardItem(QString::number((cfg->mouseButtons >> 3 & 0x1) ? 1 : 0));
    QStandardItem* RMB = new QStandardItem(QString::number((cfg->mouseButtons >> 2 & 0x1) ? 1 : 0));
    QStandardItem* MMB = new QStandardItem(QString::number((cfg->mouseButtons >> 1 & 0x1) ? 1 : 0));
    QStandardItem* MWH = new QStandardItem(QString::number((cfg->mouseButtons & 0x1) ? 1 : 0));

    items << nameItem << ipItem << secondsItem << LMB << RMB << MMB << MWH;
    treeModel->appendRow(items);
    items.clear();
}

bool Server::saveUsers()
{
    QFile usersFile("list.usr");
    if ( !usersFile.open(QIODevice::WriteOnly) )
        return false;
    QDataStream users(&usersFile);
    users << usernames;
    usersFile.close();
    return true;
}

bool Server::loadUsers()
{
    QFile usersFile("list.usr");
    if ( usersFile.exists() )
    {
        if ( !usersFile.open(QIODevice::ReadOnly) )
            return false;
        QDataStream users(&usersFile);
        users >> usernames;
        usersFile.close();
        //Check all configs in folder by user's ip
        Config* config = new Config;
        QDirIterator iter(path + "/configs", QStringList() << "*.cfg", QDir::Files);
        QString ip;
        while (iter.hasNext())
        {
            iter.next();
            ip = iter.fileName().section(".", 0, -2);
            //If user already exists => load config
            if (usernames.contains(ip))
            {
                loadConfig(*config, path + "/configs/" + ip + ".cfg");
                usersConfig.insert(ip, config);
            }
        }
        return true;
    }
    else
        return false;
}


void Server::getString(const QString str, const QString ip)
{
    //Parse string
    QString command = str.section(':', 0, 0);
    //If user online
    if (command == "ONLINE")
    {
        QString username = str.section(":", -1, -1);
        //If QHash doesn't contain new user's ip
        if ( ! usernames.contains(ip))
        {
            QString cfgPath = path + "/configs/" + ip + ".cfg";
            Config* config = new Config;
            //If config doesn't exist
            if( ! loadConfig(*config, cfgPath) )
            {
                qDebug() << "Config not found. Creating new.";
                saveConfig(*config, cfgPath);
            }
            usernames.insert(ip, username);
            usersConfig.insert(ip, config);

            //Add new user to tree model
            QList<QStandardItem*> items;
            initTreeModel(items, ip, username, config, ONLINE);
        }
        else
        {
            QList<QStandardItem*> items;
            items = treeModel->findItems(username, Qt::MatchFixedString, 0);
            if (items.size() == 1)
                items.at(0)->setIcon(QIcon(":/icons/online.png"));
            else
                qDebug() << "Error. Users have the same ip";
        }
    }
    else if (command == "OFFLINE")
    {
        QString username = str.section(":", -1, -1);
        QList<QStandardItem*> items;
        items = treeModel->findItems(username, Qt::MatchFixedString, 0);
        if (items.size() == 1)
            items.at(0)->setIcon(QIcon(":/icons/offline.png"));
        else
            qDebug() << "Error. Users have the same ip";
    }
}

void Server::setConfig(Config &cfg)
{
    //If selected nothing
    if (ui->treeUsers->currentIndex() == QModelIndex())
        return;
    else
    {
        int currentRow = ui->treeUsers->currentIndex().row();
        //TODO bindEnter
        cfg.bindEnter = false;
        // 0xLMB_RMB_MMB_MWH
        int lmb = treeModel->index(currentRow, 3).data().toBool() ? 8 : 0;
        int rmb = treeModel->index(currentRow, 4).data().toBool() ? 4 : 0;
        int mmb = treeModel->index(currentRow, 5).data().toBool() ? 2 : 0;
        int mwh = treeModel->index(currentRow, 6).data().toBool() ? 1 : 0;
        cfg.mouseButtons = lmb + rmb + mmb + mwh;
        cfg.seconds = treeModel->index(currentRow, 2).data().toInt();
    }
}

void Server::configSendClicked()
{
    //If selected nothing
    if (ui->treeUsers->currentIndex() == QModelIndex())
        return;
    else
    {
        QModelIndex ipIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 1);
        QString ip = ipIndex.data().toString();
        quint16 port = 1234;
        QString cfgPath = path + "/configs/" + ip + ".cfg";
        QString tempCfgPath = path + "/configs/" + ip + "_temp.cfg";
        QFile oldCfgFile(cfgPath);
        QFile tempCfgFile(tempCfgPath);
        Config* cfg = usersConfig.value(ip);

        setConfig(*cfg);
        fileClient->changePeer(ip, port);

        //Save temp config
        saveConfig(*cfg, tempCfgPath);
        //Send config
        fileClient->connect();
        if (fileClient->sendFile(tempCfgPath))
        {
            if (oldCfgFile.exists())
                oldCfgFile.remove();
            tempCfgFile.rename(cfgPath);
        }
        else
        {
            qDebug() << "Config not sent";
            tempCfgFile.remove();
        }
        fileClient->disconnect();
    }
}

void Server::configSaveClicked()
{
    //If selected nothing
    if (ui->treeUsers->currentIndex() == QModelIndex())
        return;
    else
    {
        QModelIndex ipIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 1);
        QString ip = ipIndex.data().toString();

        QString cfgPath = path + "/configs/" + ip + ".cfg";
        Config* cfg = usersConfig.value(ip);

        setConfig(*cfg);
        saveConfig(*cfg, cfgPath);
    }
}

void Server::configLoadClicked()
{
    qDebug() << saveUsers();
}
