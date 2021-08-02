#include <QApplication>
#include "bmsdaemon.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    bmsDaemon daeMon;
    daeMon.startServer(5329);
    return a.exec();
}
