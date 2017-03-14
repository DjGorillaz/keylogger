#ifndef CONFIG_H
#define CONFIG_H

#include <QDataStream>
#include <QDir>

struct Config
{
    //Screenshot
    quint32 seconds;
    int mouseButtons;
    bool bindEnter;
    Config();
};

//Read config
QDataStream & operator << (QDataStream& stream, Config& config);

//Write config
QDataStream & operator >> (QDataStream& stream, Config& config);

bool loadConfig(Config& config, QString defaultPath = (QString(QDir::currentPath()) + "/config.cfg") );
bool saveConfig(const Config& config, QString defaultPath = (QString(QDir::currentPath()) + "/config.cfg") );

#endif // CONFIG_H
