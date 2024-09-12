#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include<QInputDialog>
#include"privatechat.h"
#include<QMessageBox>

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit;
    m_pFriendListWidget = new QListWidget;
    m_pInputMsgLE = new QLineEdit;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户");
    m_pMsgSendPB = new QPushButton("信息发送");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;//垂直布局
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;//水平布局
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);
    connect(m_pShowOnlineUsrPB,SIGNAL(clicked(bool)),this,SLOT(showOnline()));
    connect(m_pSearchUsrPB,SIGNAL(clicked(bool)),this,SLOT(searchUsr()));
    connect(m_pFlushFriendPB,SIGNAL(clicked(bool)),this,SLOT(flushFriend()));
    connect(m_pDelFriendPB,SIGNAL(clicked(bool)),this,SLOT(delFriend()));
    connect(m_pPrivateChatPB,SIGNAL(clicked(bool)),this,SLOT(privateChat()));
    connect(m_pMsgSendPB,SIGNAL(clicked(bool)),SLOT(groupChat()));
}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if(NULL == pdu)
    {
        return ;
    }
    m_pOnline->showUsr(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    qDebug()<<"好友来了";
    if(NULL == pdu)
    {
        return ;
    }
    uint uiSize = pdu->uiMsgLen/32;
    char caName[32] = {'\0'};
    m_pFriendListWidget->clear();
    for(uint i = 0;i<uiSize;i++)
    {
        memcpy(caName,(char*)(pdu->caMsg)+i*32,32);
        m_pFriendListWidget->addItem(caName);
    }
}

void Friend::updateGroupMsg(PDU *pdu)
{
    QString strMsg = QString("%1 says: %2").arg(pdu->caData).arg((char*)(pdu->caMsg));
    m_pShowMsgTE->append(strMsg);
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()//显示在线用户
{
    if(m_pOnline->isHidden())
    {

        m_pOnline->show();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else {
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    // 获取用户输入的用户名
    m_strSearchName = QInputDialog::getText(this,"搜索","用户名：");
    if(!m_strSearchName.isEmpty())
    {
        qDebug()<<m_strSearchName;
        PDU *pdu = mkPDU(0);
        // 将用户名复制到PDU的数据部分
        memcpy(pdu->caData,m_strSearchName.toStdString().c_str(),m_strSearchName.toUtf8().size());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST; // 设置消息类型为搜索用户请求
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen); // 发送PDU
        free(pdu); // 释放PDU内存
        pdu = NULL;
    }
}

void Friend::flushFriend()
{
    qDebug()<<"flushFriend";
    QString strName = tcpclient::getInstance().loginName(); // 获取登录名
    PDU *pdu = mkPDU(0); // 创建PDU
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST; // 设置消息类型为刷新好友列表请求
    qDebug()<<pdu->uiMsgType;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.toUtf8().size()); // 将登录名复制到PDU数据部分
    tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen); // 发送PDU
    free(pdu); // 释放PDU内存
    pdu = NULL;
}

void Friend::delFriend()
{
    if(NULL != m_pFriendListWidget->currentItem()) // 判断当前选中的好友列表项是否为空
    {
        QString strFriendName =  m_pFriendListWidget->currentItem()->text(); // 获取选中的好友名
        qDebug()<<strFriendName;
        PDU *pdu = mkPDU(0); // 创建PDU
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST; // 设置消息类型为删除好友请求
        QString strSelfName = tcpclient::getInstance().loginName(); // 获取登录名

        memcpy(pdu->caData,strSelfName.toStdString().c_str(),strSelfName.toUtf8().size()); // 将登录名复制到PDU数据部分
        memcpy(pdu->caData+32,strFriendName.toStdString().c_str(),strFriendName.toUtf8().size()); // 将好友名复制到PDU数据部分
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen); // 发送PDU
        free(pdu); // 释放PDU内存
        pdu = NULL;
    }
}

void Friend::privateChat()
{
    if(NULL != m_pFriendListWidget->currentItem()) // 判断当前选中的好友列表项是否为空
    {
        QString strChatName = m_pFriendListWidget->currentItem()->text(); // 获取选中的好友名
        PrivateChat::getInstance().setChatName(strChatName); // 设置私聊窗口的聊天对象名
        if(PrivateChat::getInstance().isHidden()) // 如果私聊窗口处于隐藏状态
        {
            PrivateChat::getInstance().show(); // 显示私聊窗口
        }
    }
    else
    {
        QMessageBox::warning(this,"私聊","请选择私聊的对象"); // 弹出警告框提示用户选择私聊对象
    }
}

void Friend::groupChat()
{
    QString strMsg = m_pInputMsgLE->text(); // 获取输入框中的群聊消息内容
    if(!strMsg.isEmpty()) // 如果消息内容不为空
    {
        PDU *pdu = mkPDU(strMsg.toUtf8().size()+1); // 创建PDU，大小为消息内容长度加1
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST; // 设置消息类型为群聊请求
        QString strName = tcpclient::getInstance().loginName(); // 获取登录名
        strncpy(pdu->caData,strName.toStdString().c_str(),strName.toUtf8().size()); // 将登录名复制到PDU数据部分
        strncpy((char*)(pdu->caMsg),strMsg.toStdString().c_str(),strMsg.toUtf8().size()); // 将消息内容复制到PDU消息部分
        tcpclient::getInstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen); // 发送PDU
    }
    else
    {
        QMessageBox::warning(this,"群聊","信息不能为空"); // 弹出警告框提示用户输入群聊消息内容
    }
}
