#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include<QTextEdit>
#include<QListWidget>
#include<QLineEdit>
#include<QPushButton>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<online.h>


class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUsr(PDU *pdu);//显示所有在线用户
    void updateFriendList(PDU *pdu);//更新好友列表
    void updateGroupMsg(PDU *pdu);//更新群聊消息

    QString m_strSearchName;

    QListWidget *getFriendList();//得到好友列表

signals:
public slots:
    void showOnline();//显示在线用户
    void searchUsr();//查找用户
    void flushFriend();//刷新好友
    void delFriend();//删除好友
    void privateChat();//私聊
    void groupChat();//群聊


private:
    QTextEdit *m_pShowMsgTE;
    QListWidget *m_pFriendListWidget;
    QLineEdit *m_pInputMsgLE;
    QPushButton *m_pDelFriendPB;
    QPushButton *m_pFlushFriendPB;
    QPushButton *m_pShowOnlineUsrPB;
    QPushButton *m_pSearchUsrPB;
    QPushButton *m_pMsgSendPB;
    QPushButton *m_pPrivateChatPB;

    Online *m_pOnline;



};

#endif // FRIEND_H
