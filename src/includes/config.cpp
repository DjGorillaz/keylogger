#include "config.h"

//Read structure
QDataStream & operator << (QDataStream& stream, const Config& config)
{
    stream << config.seconds
           << config.mouseButtons
           << config.bindEnter;
    return stream;
}

//Write structure
QDataStream & operator >> (QDataStream& stream, Config& config)
{
    stream >> config.seconds
           >> config.mouseButtons
           >> config.bindEnter;
    return stream;
}

Config::Config()
{
    seconds = 0;
    mouseButtons = 0;
    bindEnter = false;
}

bool loadConfig(Config& config, const QString& defaultPath)
{
    QFile cfgFile(defaultPath);
    if ( cfgFile.exists() )
    {
        if ( !cfgFile.open(QIODevice::ReadOnly) )
            return false;
        QDataStream cfgStream(&cfgFile);
        cfgStream >> config;
        cfgFile.close();
        return true;
    }
    else
        return false;
}

bool saveConfig(const Config& config, const QString& defaultPath)
{
    QFile cfgFile(defaultPath);
    if ( !cfgFile.open(QIODevice::WriteOnly) )
        return false;
    QDataStream cfgStream(&cfgFile);
    cfgStream << config;
    cfgFile.close();
    return true;
}

