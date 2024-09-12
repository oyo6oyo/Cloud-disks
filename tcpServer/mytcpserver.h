#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H


#include <QTcpServer>
#include<QList>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();
    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);
    void resend(const char *pername,const char* name,PDU *pdu);

public slots:
    void deletesocket(MyTcpSocket *mysocket);
private:
    QList<MyTcpSocket*> m_tcpSocKetList;//存储登录用户
};

#endif // MYTCPSERVER_H
