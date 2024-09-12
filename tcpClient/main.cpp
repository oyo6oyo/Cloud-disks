#include "tcpclient.h"

#include <QApplication>

#include "sharefile.h"
// #include "book.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // tcpclient w;
    // w.show();
    tcpclient::getInstance().show();

    // ShareFile w;
    // w.text();
    // w.show();

    // Book w;
    // w.show();
    return a.exec();
}
