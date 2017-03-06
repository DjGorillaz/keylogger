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

void initDefaultConfig(Config& config)
{
    config.seconds = 0;
    config.mouseButtons = 0;
    config.bindEnter = false;
}

bool loadConfig(Config& config, QString defaultPath)
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

bool saveConfig(const Config& config, QString defaultPath)
{
    QFile cfgFile(defaultPath);
    if ( !cfgFile.open(QIODevice::WriteOnly) )
        return false;
    QDataStream cfgStream(&cfgFile);
    cfgStream << config;
    cfgFile.close();
    return true;
}

