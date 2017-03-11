#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QMap>
#include <QTreeWidget>

#include "config.h"
#include "fileserver.h"
#include "fileclient.h"

#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QDirIterator>
#include <QAbstractProxyModel>

namespace Ui {
class Server;
}

class Server : public QMainWindow
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = 0);
    ~Server();

private slots:
    void getString(QString str, QString ip);
    void configSendClicked();
    void configSaveClicked();
    void configLoadClicked();

private:
    Ui::Server *ui;

    void setForm(Config& cfg);
    void setConfig(Config& cfg);
    void setupModels();
    bool saveUsers();
    bool loadUsers();

    QStandardItemModel* treeModel;
    QDataWidgetMapper* treeMapper;
    QStandardItemModel* uiModel;
    QDataWidgetMapper* uiMapper;

    FileServer* fileServer;
    FileClient* fileClient;
    QHash<QString, Config*> usersConfig;
    QHash<QString, QString> usernames;


};

#endif // SERVER_H
