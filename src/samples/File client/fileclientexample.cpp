#include "fileclient.h"
//#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    FileClient* client1 = new FileClient(0, "127.0.0.1", 12345);
    char ch = 'y';
    QString str;
    QTextStream stream(stdin);
    if(client1->connect())
    {
        do
        {
            qDebug() << "1 - send file\n2 - send string\ne - exit\n";
            //std::cin >> ch;
            switch (ch = getchar()) {
            case '1':
                qDebug() << "\nEnter file name";
                stream.skipWhiteSpace();
                str = stream.readLine();
                qDebug() << str;
                if (client1->sendFile(str))
                {
                    qDebug() << "File transmitted!";
                }
                break;
            case '2':
                qDebug() << "Enter string";
                stream.skipWhiteSpace();
                str = stream.readLine();
                qDebug() << str;
                if (client1->sendStr(str) )
                    qDebug() << "string sent";
                break;
            default:
                break;
            }
        } while (ch != 'e');
    }

    delete client1;

    return a.exec();
}
