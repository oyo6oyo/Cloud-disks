#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include"opedb.h"
#include<QFile>
#include <QTimer>
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getName();//验证是否删除
    void copyDir(QString strSrcDir,QString strDestDir);
signals:
    void offline(MyTcpSocket *mysocket);
public slots:
    void recvMsg();//消息响应函数
    void clientOffline();//客户端窗口关闭online改变函数
    void sendFileToClient();
private:
    QString m_strName;
    QFile m_file;
    qint64 m_iTotal;
    qint64 m_iRecved;
    bool m_bUpload;
    QTimer *m_pTimer;
};

#endif // MYTCPSOCKET_H
