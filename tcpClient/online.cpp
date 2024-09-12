#include "online.h" // 引入在线界面头文件
#include "ui_online.h" // 引入在线界面UI头文件
#include "tcpclient.h" // 引入TCP客户端头文件
#include<QDebug> // 引入Qt调试输出头文件

Online::Online(QWidget *parent)
    : QWidget(parent) // 构造函数，初始化父窗口
    , ui(new Ui::Online) // 创建UI对象
{
    ui->setupUi(this); // 设置UI界面
}

Online::~Online()
{
    delete ui; // 析构函数，删除UI对象
}

void Online::showUsr(PDU *pdu)
{
    if(NULL == pdu) // 如果传入的PDU指针为空，直接返回
    {
        return ;
    }
    uint uiSize = pdu->uiMsgLen/32; // 计算在线用户数量
    char caTmp[32]; // 临时字符数组，用于存储用户名
    ui->online_lw->clear(); // 清空在线用户列表
    for(int i=0;i<uiSize;i++) // 遍历所有在线用户
    {
        memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32); // 复制用户名到临时数组
        ui->online_lw->addItem(caTmp); // 将用户名添加到在线用户列表中
    }
}

void Online::on_pushButton_clicked()
{
    QListWidgetItem *pItem = ui->online_lw->currentItem(); // 获取当前选中的用户项
    QString strPerUsrName = pItem->text(); // 获取选中用户的用户名
    QString strLoginName = tcpclient::getInstance().loginName(); // 获取登录名
    PDU *pdu = mkPDU(0); // 创建一个新的PDU对象
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST; // 设置消息类型为添加好友请求
    memcpy(pdu->caData,strPerUsrName.toStdString().c_str(),strPerUsrName.toUtf8().size()); // 将选中用户的用户名复制到PDU数据部分
    memcpy(pdu->caData+32,strLoginName.toStdString().c_str(),strLoginName.toUtf8().size()); // 将登录名复制到PDU数据部分
    tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen); // 通过TCP套接字发送PDU
    free(pdu); // 释放PDU对象内存
    pdu = NULL; // 将PDU指针置空
}
