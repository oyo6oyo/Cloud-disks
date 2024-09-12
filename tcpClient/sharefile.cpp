#include "sharefile.h"
#include "tcpclient.h"
#include "opewidget.h"

ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    // 创建全选按钮和取消选择按钮
    m_pSelectAllPB = new QPushButton("全选");
    m_CancelSelectPB = new QPushButton("取消选择");

    // 创建确定按钮和取消按钮
    m_pOKPB = new QPushButton("确定");
    m_CancelPB = new QPushButton("取消");

    // 创建滚动区域和好友列表窗口
    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);
    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    m_pButtonGroup->setExclusive(false);

    // 创建顶部布局，包含全选按钮、取消选择按钮和占位符
    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_CancelSelectPB);
    pTopHBL->addStretch();

    // 创建底部布局，包含确定按钮和取消按钮
    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_CancelPB);

    // 创建主布局，包含顶部布局、滚动区域和底部布局
    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);

    // 连接信号和槽函数
    connect(m_CancelSelectPB,SIGNAL(clicked(bool)),this,SLOT(cancelSelect()));
    connect(m_pSelectAllPB,SIGNAL(clicked(bool)),this,SLOT(selectAll()));
    connect(m_pOKPB,SIGNAL(clicked(bool)),this,SLOT(okShare()));
    connect(m_CancelPB,SIGNAL(clicked(bool)),this,SLOT(cancelShare()));
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance; // 单例模式，确保只有一个实例
    return instance;
}

void ShareFile::text()
{
    QVBoxLayout *p = new QVBoxLayout(m_pFriendW);
    QCheckBox *pCB = NULL;
    for(int i=0;i<15;i++)
    {
        pCB = new QCheckBox("jack"); // 创建复选框并设置文本为"jack"
        p->addWidget(pCB); // 将复选框添加到布局中
        m_pButtonGroup->addButton(pCB); // 将复选框添加到按钮组中
    }
    m_pSA->setWidget(m_pFriendW); // 将布局设置为滚动区域的窗口部件
}

void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if(pFriendList == NULL)
    {
        return ;
    }
    QAbstractButton *tmp =NULL;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons(); // 获取之前的好友列表
    for(int i = 0; i < preFriendList.size();i++)
    {
        tmp = preFriendList[i]; // 获取当前好友
        m_pFriendWVBL->removeWidget(tmp); // 从布局中移除当前好友
        m_pButtonGroup->removeButton(tmp); // 从按钮组中移除当前好友
        preFriendList.removeOne(tmp); // 从列表中移除当前好友
        delete tmp; // 删除当前好友对象
        tmp = NULL; // 重置指针
    }
    QCheckBox *pCB = NULL;
    for(int i=0;i<pFriendList->count();i++)
    {
        pCB = new QCheckBox(pFriendList->item(i)->text()); // 创建复选框并设置文本为好友名
        m_pFriendWVBL->addWidget(pCB); // 将复选框添加到布局中
        m_pButtonGroup->addButton(pCB); // 将复选框添加到按钮组中
    }
    m_pSA->setWidget(m_pFriendW); // 将布局设置为滚动区域的窗口部件
}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons(); // 获取所有复选框列表
    for(int i = 0;i < cbList.size();i ++)
    {
        if(cbList[i]->isChecked()) // 如果复选框被选中
        {
            cbList[i]->setChecked(false); // 取消选中状态
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons(); // 获取所有复选框列表
    for(int i = 0;i < cbList.size();i ++)
    {
        if(!cbList[i]->isChecked()) // 如果复选框未被选中
        {
            cbList[i]->setChecked(true); // 设置为选中状态
        }
    }
}

void ShareFile::okShare()
{
    QString strName = tcpclient::getInstance().loginName(); // 获取登录名
    QString strCurPath = tcpclient::getInstance().curPath(); // 获取当前路径
    QString strShareFileName = OpeWidget::getIntance().getBook()->getShareFileName(); // 获取分享文件名

    QString strPath = strCurPath +'/' + strShareFileName; // 拼接完整路径
    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons(); // 获取所有复选框列表
    int num = 0;
    for(int i = 0;i < cbList.size();i ++)
    {
        if(cbList[i]->isChecked()) // 如果复选框被选中
        {
            num++; // 计数器加一
        }
    }
    PDU *pdu = mkPDU(32*num + strPath.toUtf8().size()+1); // 创建PDU对象，大小为选中复选框数量乘以32加上路径长度加1
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST; // 设置消息类型为分享文件请求
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),num); // 将登录名和选中复选框数量写入PDU数据部分
    qDebug()<<"客户端分享"; // 输出调试信息
    int j = 0;
    for(int i=0;i<cbList.size();i++)
    {
        if(cbList[i]->isChecked()) // 如果复选框被选中
        {
            memcpy((char*)(pdu->caMsg)+j*32,cbList[i]->text().toStdString().c_str(),cbList[i]->text().toUtf8().size()); // 将选中的好友名复制到PDU消息部分
            j++; // 计数器加一
        }
    }
    memcpy((char*)(pdu->caMsg)+num*32,strPath.toStdString().c_str(),strPath.toUtf8().size()); // 将完整路径复制到PDU消息部分
    tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen); // 发送PDU数据包
    free(pdu); // 释放PDU对象内存
    pdu = NULL; // 重置指针
}

void ShareFile::cancelShare()
{
    hide(); // 隐藏窗口
}
