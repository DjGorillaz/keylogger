#include <QCoreApplication>
#include "mousehookWinApi.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QObject::connect(&MouseHook::instance(), SIGNAL(mouseClicked()),
                     &MouseHook::instance(), SLOT(makeScreenshot()));
    return a.exec();
}
