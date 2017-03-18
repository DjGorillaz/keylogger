#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QDirIterator>

#include "config.h"
#include "fileserver.h"
#include "fileclient.h"

enum state {
    OFFLINE,
    ONLINE
};

namespace Ui {
class Server;
}

class Server : public QMainWindow
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = 0, const QString& path = QDir::currentPath());
    ~Server();

private slots:
    void getString(const QString str, const QString ip);
    void configSendClicked();
    void configSaveClicked();
    void configLoadClicked();

private:
    void setConfig(Config& cfg);
    void setupModels();
    bool saveUsers();
    bool loadUsers();
    void initTreeModel(QList<QStandardItem*> &items, const QString &ip, const QString &username, const Config *cfg, const state& st);

    QStandardItemModel* treeModel;
    QDataWidgetMapper* uiMapper;

    QString path;
    FileServer* fileServer;
    FileClient* fileClient;
    QHash<QString, Config*> usersConfig;
    QHash<QString, QString> usernames;
    Ui::Server *ui;
};

#endif // SERVER_H
