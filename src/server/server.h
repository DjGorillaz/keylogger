#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QMap>

#include "config.h"
#include "fileserver.h"
#include "fileclient.h"


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

    FileServer* fileServer;
    FileClient* fileClient;
    QHash<QString, Config*> usersConfig;
    QHash<QString, QString> usernames;
};

#endif // SERVER_H
