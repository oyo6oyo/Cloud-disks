#include "tcpserver.h"
#include "opedb.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpeDB::getInstance().init();
    tcpserver w;
    w.show();
    return a.exec();
}
