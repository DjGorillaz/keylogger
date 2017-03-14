#include "server.h"
#include "ui_server.h"

//https://www.iconfinder.com/icons/46254/active_approval_online_status_icon#size=16
//https://www.iconfinder.com/icons/46252/busy_offline_status_icon#size=16
//Yusuke Kamiyamane
//License: Creative Commons (Attribution 3.0 Unported)

void Server::setupModels()
{
    treeModel = new QStandardItemModel(this);
    treeModel->setColumnCount(2);
    treeModel->setHeaderData(0, Qt::Horizontal, "username");
    treeModel->setHeaderData(1, Qt::Horizontal, "ip");

    uiModel = new QStandardItemModel(this);
    uiModel->setColumnCount(6);

    QList<QStandardItem*> items;
    QStandardItem* nameItem;
    QStandardItem* ipItem;
    QStandardItem* secondsItem;
    QStandardItem* LMB;
    QStandardItem* RMB;
    QStandardItem* MMB;
    QStandardItem* MWH;
    int seconds;
    int buttons;

    QHash<QString, QString>::iterator iter = usernames.begin();
    Config* cfg;
    QString ip;
    while (iter != usernames.end())
    {
        //First model
        ip = iter.key();
        ipItem = new QStandardItem(ip);
        nameItem = new QStandardItem(QIcon("offline.png"), iter.value());
        //Set not editable item
        ipItem->setFlags(ipItem->flags() & ~Qt::ItemIsEditable);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        items << nameItem << ipItem;
        treeModel->appendRow(items);
        items.clear();

        //load config
        if (usersConfig.contains(ip))
            cfg = usersConfig.value(ip);
        else
        {
            cfg = new Config();
            loadConfig(*cfg, "./configs/" + ip + ".cfg");
        }

        //Second model
        ipItem = new QStandardItem(ip);
        seconds = cfg->seconds;
        buttons = cfg->mouseButtons;

        secondsItem = new QStandardItem(QString::number(seconds));
        LMB = new QStandardItem(QString::number((buttons >> 3 & 0x1) ? 1 : 0));
        RMB = new QStandardItem(QString::number((buttons >> 2 & 0x1) ? 1 : 0));
        MMB = new QStandardItem(QString::number((buttons >> 1 & 0x1) ? 1 : 0));
        MWH = new QStandardItem(QString::number((buttons & 0x1) ? 1 : 0));

        items << ipItem << secondsItem << LMB << RMB << MMB << MWH;
        uiModel->appendRow(items);
        items.clear();
        ++iter;
    }
}

Server::Server(QWidget *parent) :
    QMainWindow(parent),
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

    //Setup model, tree and mapper
    setupModels();
    ui->treeUsers->setModel(treeModel);

/*
    QItemSelectionModel *selectionModel = ui->treeUsers->selectionModel();

    QModelIndex topLeft = treeModel->index(0, 0); //,QModelIndex());
    QModelIndex bottomRight = treeModel->index(0, 0); //, QModelIndex());
    QItemSelection columnSelection;
    columnSelection.select(topLeft, bottomRight);
    selectionModel->select(columnSelection,
        QItemSelectionModel::Select | QItemSelectionModel::Columns);

    QAbstractProxyModel* proxy = new QAbstractProxyModel(this);
    */

    treeMapper = new QDataWidgetMapper(this);
    treeMapper->setModel(treeModel);

    uiMapper = new QDataWidgetMapper(this);
    uiMapper->setModel(uiModel);
    uiMapper->addMapping(ui->spinSeconds, 1, "value");
    uiMapper->addMapping(ui->checkLMB, 2);
    uiMapper->addMapping(ui->checkRMB, 3);
    uiMapper->addMapping(ui->checkMMB, 4);
    uiMapper->addMapping(ui->checkMWH, 5);

    connect(ui->treeUsers, &QTreeView::pressed,
            [this](const QModelIndex& index) {
                //TODO fix setCurrentModelIndex
                uiMapper->setCurrentIndex(index.row());
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
            }
            );
    //treeMapper->toFirst();
    //uiMapper->toFirst();

    fileServer = new FileServer(this, 12345);
    fileClient = new FileClient(this, "", 0);
    fileServer->start();
    QDir configDir;
    configDir.mkdir("configs");
    connect(fileServer, &FileServer::stringRecieved, [this](QString str, QString ip) { this->getString(str, ip); });

    //Connect buttons clicks
    connect(ui->buttonSendConfig, &QPushButton::clicked, this, &Server::configSendClicked);
    connect(ui->buttonSaveConfig, &QPushButton::clicked, this, &Server::configSaveClicked);
    connect(ui->buttonLoadConfig, &QPushButton::clicked, this, &Server::configLoadClicked);
}

Server::~Server()
{
    saveUsers();
    //TODO
    delete ui;
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
        QDirIterator iter("./configs/", QStringList() << "*.cfg", QDir::Files);
        QString ip;
        while (iter.hasNext())
        {
            iter.next();
            ip = iter.fileName().section(".", 0, -2);
            //If user already exists then load config
            if (usernames.contains(ip))
            {
                loadConfig(*config, "./configs/" + ip + ".cfg");
                usersConfig.insert(ip, config);
            }
        }
        return true;
    }
    else
        return false;
}


void Server::getString(QString str, QString ip)
{
    //parse string
    QString command = str.section(':', 0, 0);
    //If user online
    if (command == "ONLINE")
    {
        QString userName = str.section(":", -1, -1);
        //If QHash doesn't contain new user's ip
        if ( ! usernames.contains(ip))
        {
            QString path = QDir::currentPath() + "/configs/" + ip + ".cfg";

            Config* config = new Config;
            //If config doesn't exist
            if( ! loadConfig(*config, path) )
            {
                qDebug() << "Config not found. Creating new.";
                saveConfig(*config, path);
            }
            usernames.insert(ip, userName);
            usersConfig.insert(ip, config);

            //Add user to Tree
            QList<QStandardItem*> items;
            QStandardItem* nameItem = new QStandardItem(QIcon("online.png"), userName);
            QStandardItem* ipItem = new QStandardItem(ip);
            //Set not editable item
            ipItem->setFlags(ipItem->flags() & ~Qt::ItemIsEditable);
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
            items << nameItem << ipItem;
            treeModel->appendRow(items);
            setForm(*config);
        }
        else
        {
            QList<QStandardItem*> items;
            QTreeModel* found;
            items = treeModel->findItems(userName, Qt::MatchFixedString, 0);
            if (items.size() == 1)
                items.at(0)->setIcon(QIcon("online.png"));
            else
                qDebug() << "Error. Users have the same ip";

        }
    }
    else if (command == "OFFLINE")
    {
        //TODO
        /*
        QList<QTreeWidgetItem*> userList = ui->treeUsers->findItems(ip, Qt::MatchFixedString, 1);
        if (userList.size() > 1)
            qDebug() << "Error. 2 or more users have the same ip";
        else
        {

        }
        */
        //TODO
    }
}

void Server::setForm(Config &cfg)
{
    ui->spinSeconds->setValue(static_cast<int>(cfg.seconds));

    int buttons = cfg.mouseButtons;

    // 0xLMB_RMB_MMB_MWH
    (buttons >> 3 & 0x1) ? ui->checkLMB->setCheckState(Qt::Checked) :
                           ui->checkLMB->setCheckState(Qt::Unchecked);
    (buttons >> 2 & 0x1) ? ui->checkRMB->setCheckState(Qt::Checked) :
                           ui->checkRMB->setCheckState(Qt::Unchecked);
    (buttons >> 1 & 0x1) ? ui->checkMMB->setCheckState(Qt::Checked) :
                           ui->checkMMB->setCheckState(Qt::Unchecked);
    (buttons & 0x1) ? ui->checkMWH->setCheckState(Qt::Checked) :
                      ui->checkMWH->setCheckState(Qt::Unchecked);

    //TODO: bindEnter
}

void Server::setConfig(Config &cfg)
{
    //TODO bindEnter
    cfg.bindEnter = false;
    // 0xLMB_RMB_MMB_MWH
    int lmb = (ui->checkLMB->checkState() == Qt::Checked) ? 8 : 0;
    int rmb = (ui->checkRMB->checkState() == Qt::Checked) ? 4 : 0;
    int mmb = (ui->checkMMB->checkState() == Qt::Checked) ? 2 : 0;
    int mwh = (ui->checkMWH->checkState() == Qt::Checked) ? 1 : 0;
    cfg.mouseButtons = lmb + rmb + mmb + mwh;
    cfg.seconds = ui->spinSeconds->value();
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

        QModelIndex nameIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 0);
        QString userName = nameIndex.data().toString();
        QString cfgPath = QDir::currentPath() + "/configs/" + ip + ".cfg";
        Config* cfg = usersConfig.value(ip);
    
        //TODO: error while sending config
        //save as temp.cfg ?? before get bool result
    
        fileClient->changePeer(ip, port);
        setConfig(*cfg);
        saveConfig(*cfg, cfgPath);
        fileClient->connect();
        fileClient->sendFile(cfgPath);
        fileClient->disconnect();
    }
}

void Server::configSaveClicked()
{
    if (ui->treeUsers->currentIndex() == QModelIndex())
        return;
    else
    {
        QModelIndex ipIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 1);
        QString ip = ipIndex.data().toString();

        QModelIndex nameIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 0);
        QString userName = nameIndex.data().toString();
        QString cfgPath = QDir::currentPath() + "/configs/" + ip + ".cfg";
        Config* cfg = usersConfig.value(ip);

        setConfig(*cfg);
        saveConfig(*cfg, cfgPath);
    }
}

void Server::configLoadClicked()
{
    qDebug() << saveUsers();
}
