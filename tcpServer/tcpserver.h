#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include "mytcpserver.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class tcpserver;
}
QT_END_NAMESPACE

class tcpserver : public QWidget
{
    Q_OBJECT

public:
    tcpserver(QWidget *parent = nullptr);
    ~tcpserver();
    void loadConfig();

private:
    Ui::tcpserver *ui;
    QString m_strIP;
    quint16 m_usPort;
};
#endif // TCPSERVER_H
