#include "mytcpserver.h"
#include<QDebug>

MyTcpServer::MyTcpServer() // 构造函数
{

}

MyTcpServer &MyTcpServer::getInstance() // 获取单例对象
{
    static MyTcpServer instance; // 静态局部变量，只会被初始化一次
    return instance; // 返回单例对象引用
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor) // 处理新的连接
{
    MyTcpSocket *pTcpSocket = new MyTcpSocket; // 创建一个新的MyTcpSocket对象
    pTcpSocket->setSocketDescriptor(socketDescriptor); // 关联套接字描述符
    qDebug()<<"new client connect"; // 输出调试信息
    m_tcpSocKetList.append(pTcpSocket); // 将新创建的套接字添加到列表中
    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),this,SLOT(deletesocket(MyTcpSocket*))); // 关联信号槽，当客户端离线时触发deletesocket槽函数

}

void MyTcpServer::resend(const char *pername,const char* name, PDU *pdu) // 重新发送消息
{
    if(NULL == pername || NULL == pdu ||NULL == name) // 检查参数是否为空
    {
        return ;
    }
    QString strName = pername; // 将pername转换为QString类型
    qDebug()<<"大小"<<pername<<":to:"<<name; // 输出调试信息
    for(int i=0;i<m_tcpSocKetList.size();i++) // 遍历套接字列表
    {
        if(strName == m_tcpSocKetList.at(i)->getName()) // 如果找到了对应的套接字
        {
            strncpy(pdu->caData+32,pername,32); // 将pername复制到PDU数据中的指定位置
            strncpy(pdu->caData,name,32); // 将name复制到PDU数据中的指定位置
            m_tcpSocKetList.at(i)->write((char*)pdu,pdu->uiPDULen); // 向套接字写入PDU数据
            break;
        }
    }

}

void MyTcpServer::deletesocket(MyTcpSocket *mysocket) // 删除客户端对象
{
    // 迭代器遍历
    QList<MyTcpSocket*>::iterator iter = m_tcpSocKetList.begin();

    for(;iter!=m_tcpSocKetList.end();iter++) // 遍历套接字列表
    {
        if(mysocket == *iter) // 如果找到了要删除的套接字
        {
            (*iter) -> deleteLater(); // 延迟释放空间，避免异常结束
            *iter = NULL; // 将指针置空
            m_tcpSocKetList.erase(iter); // 从列表中删除指针
            break;
        }
    }
    for(int i=0;i<m_tcpSocKetList.size();i++) // 验证是否删除成功
    {
        qDebug()<<":"<<m_tcpSocKetList.at(i)->getName(); // 输出调试信息
    }

}
