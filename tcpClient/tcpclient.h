#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include<QFile>
#include<QTcpSocket>
#include "opewidget.h"
#include "protocol.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class tcpclient;
}
QT_END_NAMESPACE

class tcpclient : public QWidget
{
    Q_OBJECT

public:
    tcpclient(QWidget *parent = nullptr);
    ~tcpclient();
    void loadConfig();//连接服务器

    static tcpclient &getInstance();//单例
    QTcpSocket &getTcpSocket();
    QString loginName();//返回登录名
    QString curPath();//返回当前路径
    void setCurPath(QString strCurPath);//设置当前路径

public slots:
    void showConnect();//是否连接服务器
    void recvMsg();//服务器消息回复


private slots:
    // void on_sendpb_clicked();

    void on_login_pb_clicked();//登录

    void on_regist_pb_clicked();//注册

    void on_cancel_pb_clicked();//注销

private:
    Ui::tcpclient *ui;

    QString m_strIP;
    quint16 m_usPort;
    //连接服务器，和服务器进行交互
    QTcpSocket m_tcpSocket;
    QString m_strLoginName;
    QString m_strCurPath;
    QFile m_file;
};
#endif // TCPCLIENT_H
