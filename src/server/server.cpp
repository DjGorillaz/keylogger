#include "server.h"
#include "ui_server.h"

Server::Server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);
    //Disable buttons
    ui->buttonLoadConfig->setEnabled(false);
    ui->buttonSaveConfig->setEnabled(false);
    ui->buttonSendConfig->setEnabled(false);

    //TODO: disable all GUI before select TreeItem

    fileServer = new FileServer(0, 12345);
    fileClient = new FileClient(0, "", 0);
    fileServer->start();
    QDir configDir;
    configDir.mkdir("configs");
    connect(fileServer, &FileServer::stringRecieved, [this](QString str, QString ip) { this->getString(str, ip); });

    //Connect buttons clicks
    connect(ui->buttonSendConfig, &QPushButton::clicked, this, &Server::configSendClicked);
    connect(ui->buttonSaveConfig, &QPushButton::clicked, this, &Server::configSaveClicked);

    //Connect tree item selection and buttons
    //void QTreeWidget::itemActivated(QTreeWidgetItem *item, int column);
    connect(ui->treeUsers, &QTreeWidget::itemPressed,
            [this](QTreeWidgetItem* item) {
                if(item->parent() == nullptr)
                {
                    ui->buttonLoadConfig->setEnabled(true);
                    ui->buttonSaveConfig->setEnabled(true);
                    ui->buttonSendConfig->setEnabled(true);
                    Config* cfg = usersConfig.value(item->text(1));
                    setForm(*cfg);
                }
            } );
}

Server::~Server()
{
    delete ui;
}

void Server::getString(QString str, QString ip)
{
    //parse string
    QString command = str.section(':', 0, 0);
    //If user online
    if (command == "ONLINE")
    {
        //If QHash doesn't contain new user's ip
        if ( ! usernames.contains(ip))
        {
            QString userName = str.section(":", -1, -1);
            QString path = QDir::currentPath() + "/configs/" + userName + ".cfg";

            Config* config = new Config;
            //If config doesn't exist
            if( ! loadConfig(*config, path) )
            {
                qDebug() << "Config not found. Creating new.";
                initDefaultConfig(*config);
                saveConfig(*config, path);
            }
            usernames.insert(ip, userName);
            usersConfig.insert(ip, config);

            //Add user to TreeWidget
            QTreeWidgetItem *user = new QTreeWidgetItem(ui->treeUsers);
            user->setText(0, userName);
            user->setText(1, ip);
            setForm(*config);
        }
    }
    else if (command == "OFFLINE")
    {
        //TODO
    }
}

void Server::setForm(Config &cfg)
{
    ui->spinSeconds->setValue(static_cast<int>(cfg.seconds));

    int buttons = cfg.mouseButtons;

    //qDebug() << buttons; //((buttons >> 4) & 0x1);

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
    //TODO bindEnter:
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
    //TODO: check Select more then one item
    if (ui->treeUsers->selectedItems().size() == 1)
    {
        QTreeWidgetItem* item = ui->treeUsers->selectedItems().at(0);
        QString ip = item->text(1);
        quint16 port = 1234;

        QString userName = item->text(0);
        QString cfgPath = QDir::currentPath() + "/configs/" + userName + ".cfg";
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
    if (ui->treeUsers->selectedItems().size() == 1)
    {
        QTreeWidgetItem* item = ui->treeUsers->selectedItems().at(0);
        QString ip = item->text(1);
        QString userName = item->text(0);
        QString cfgPath = QDir::currentPath() + "/configs/" + userName + ".cfg";
        Config* cfg = usersConfig.value(ip);
        setConfig(*cfg);
        saveConfig(*cfg, cfgPath);
    }
}

void Server::configLoadClicked()
{

}
