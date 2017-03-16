#include "server.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server w(0, "/home/djgorillaz/server");
    w.show();

    return a.exec();
}
