#include "screenshotQt.h"

int main(int argc, char *argv[])
{
    //QCoreApplication a(argc, argv);
    QApplication a(argc, argv);

    Screenshot scr1;
    //scr1.makeScreenshot();
    scr1.changeTimer(4);


    return a.exec();
}
