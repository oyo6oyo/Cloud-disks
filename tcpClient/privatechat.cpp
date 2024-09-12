#include "privatechat.h" // 引入PrivateChat类的头文件
#include "ui_privatechat.h" // 引入UI界面的头文件
#include "protocol.h" // 引入协议相关的头文件
#include "tcpclient.h" // 引入TCP客户端的头文件
#include<QMessageBox> // 引入消息框的头文件

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent) // 构造函数，初始化父窗口
    , ui(new Ui::PrivateChat) // 创建UI对象
{
    ui->setupUi(this); // 设置UI界面
}

PrivateChat::~PrivateChat()
{
    delete ui; // 析构函数，删除UI对象
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance; // 静态实例，保证只有一个实例存在
    return instance; // 返回实例
}

void PrivateChat::setChatName(QString strName)
{
    m_strChatName = strName; // 设置聊天对象的名字
    m_strLoginName = tcpclient::getInstance().loginName(); // 获取登录名
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if(NULL == pdu) // 如果PDU为空，直接返回
    {
        return;
    }
    char caSendname[32] = {'\0'}; // 发送者名字数组
    memcpy(caSendname,pdu->caData,32); // 复制发送者名字到数组
    QString strMsg = QString("%1 says: %2").arg(caSendname).arg((char*)(pdu->caMsg)); // 格式化消息字符串
    ui->showMsg_te->append(strMsg); // 在文本框中追加消息
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text(); // 获取输入框中的文本
    ui->inputMsg_le->text().clear(); // 清空输入框
    if(!strMsg.isEmpty()) // 如果文本不为空
    {
        PDU *pdu = mkPDU(strMsg.toUtf8().size()+1); // 创建PDU对象
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST; // 设置消息类型为私聊请求
        memcpy(pdu->caData,m_strLoginName.toStdString().c_str(),m_strLoginName.toUtf8().size()); // 复制登录名到PDU数据部分
        memcpy(pdu->caData+32,m_strChatName.toStdString().c_str(),m_strChatName.toUtf8().size()); // 复制聊天对象名字到PDU数据部分
        strcpy((char*)pdu->caMsg,strMsg.toStdString().c_str()); // 复制消息内容到PDU消息部分
        qDebug()<<"可惜"; // 输出调试信息
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen); // 通过TCP套接字发送PDU
        free(pdu); // 释放PDU对象内存
        pdu = NULL; // 将PDU指针置空
    }
    else // 如果文本为空
    {
        QMessageBox::warning(this,"私聊","发送的信息不能为空"); // 弹出警告框提示用户
    }
}
