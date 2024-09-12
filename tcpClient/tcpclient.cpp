#include "tcpclient.h"
#include "ui_tcpclient.h"
#include<QFile>
#include<QDebug>
#include<QMessageBox>
#include<QHostAddress>
#include"privatechat.h"
tcpclient::tcpclient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::tcpclient)
{
    ui->setupUi(this);
    resize(500,300);//重新设置窗口大小
    loadConfig();//读取服务器地址
    //连接信号槽
    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));//连接服务器成功后显示连接成功窗口
    connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));//有数据可读时调用消息处理函数
    //连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);

}

tcpclient::~tcpclient()
{
    delete ui;
}

void tcpclient::loadConfig()//读取服务器地址
{
    QFile file(":/client.config"); // 打开配置文件
    if(file.open(QIODevice::ReadOnly)) // 如果文件打开成功
    {
        QByteArray baData = file.readAll(); // 读取文件内容到字节数组
        QString strData = baData.toStdString().c_str(); // 将字节数组转换为字符串
        file.close(); // 关闭文件
        strData.replace("\n"," ");//windows是"\r\n" // 替换换行符为空格
        QStringList strlist = strData.split(" "); // 以空格分割字符串
        qDebug()<<"";
        m_strIP = strlist.at(0); // 获取IP地址
        m_usPort = strlist.at(1).toUShort(); // 获取端口号
        qDebug()<<"ip:"<<m_strIP<<"port:"<<m_usPort; // 输出IP和端口信息
    }
    else
    {
        QMessageBox::critical(this,"open config","open config failed");//读取失败处理
    }
}

tcpclient &tcpclient::getInstance()
{
    static tcpclient instance; // 创建单例对象
    return instance; // 返回单例对象引用
}

QTcpSocket &tcpclient::getTcpSocket()
{
    return m_tcpSocket; // 返回TCP套接字引用
}

QString tcpclient::loginName()
{
    return m_strLoginName; // 返回登录名
}

QString tcpclient::curPath()
{
    return m_strCurPath; // 返回当前路径
}

void tcpclient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath; // 设置当前路径
}

void tcpclient::showConnect()//连接成功窗口
{
    QMessageBox::information(this,"连接服务器","连接服务器成功"); // 显示连接成功的提示框
}

void tcpclient::recvMsg()//消息处理函数
{
    if(!OpeWidget::getIntance().getBook()->getDownloadStatus())
    {
        uint  uiPDULen = 0;
        m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));//读取总协议数据单元大小
        uint uiMsgLen = uiPDULen - sizeof(PDU);//得到实际消息长度（int csMsg[];定义时并没有分配内存，动态分配）
        PDU *pdu = mkPDU(uiMsgLen);//消息数据对象
        m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));//前者为消息类型开始，后者为剩下的数据

        switch(pdu->uiMsgType)//选择消息类型
        {
        case ENUM_MSG_TYPE_REGIST_RESPOND://注册响应
        {
            qDebug()<<"兄弟"<<pdu->caData;
            if(0 == strcmp(pdu->caData,REGIST_OK))
            {
                QMessageBox::information(this,"注册",REGIST_OK);
            }
            else if(0 == strcmp(pdu->caData,REGIST_FAILED))
            {
                QMessageBox::warning(this,"注册",REGIST_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND://登录回复
        {
            qDebug()<<"LOGIN"<<pdu->caData;//name和pwd
            if(0 == strcmp(pdu->caData,LOGIN_OK))
            {
                m_strCurPath = QString("../../StoreDb/%1").arg(m_strLoginName);
                QMessageBox::information(this,"登录",LOGIN_OK);
                OpeWidget::getIntance().show();
                this->hide();
            }
            else if(0 == strcmp(pdu->caData,LOGIN_FAILED))
            {
                QMessageBox::warning(this,"登录",LOGIN_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND://在线用户回复
        {
            OpeWidget::getIntance().getFriend()->showAllOnlineUsr(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND://搜索用户回复
        {
            if(0 == strcmp(SEARCH_USR_NO,pdu->caData))
            {
                QMessageBox::information(this,"搜索",QString("%1 not exsit").arg(OpeWidget::getIntance().getFriend()->m_strSearchName));
            }
            else if(0 == strcmp(SEARCH_USR_ONLINE,pdu->caData))
            {
                QMessageBox::information(this,"搜索",QString("%1 online").arg(OpeWidget::getIntance().getFriend()->m_strSearchName));
            }
            else if(0 == strcmp(SEARCH_USR_OFFLINE,pdu->caData))
            {
                QMessageBox::information(this,"搜索",QString("%1 offline").arg(OpeWidget::getIntance().getFriend()->m_strSearchName));
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPerName[32] = {'\0'};
            strncpy(caPerName,pdu->caData+32,32);
            strncpy(caName,pdu->caData,32);
            int ret = QMessageBox::information(this,"添加好友",QString("%1 want to add you(%2) as friend ?").arg(caPerName).arg(caName),QMessageBox::Yes,QMessageBox::No);
            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData,pdu->caData,32);
            memcpy(respdu->caData+32,pdu->caData+32,32);
            if(QMessageBox::Yes == ret)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_AGREE_REQUEST;
            }
            else
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_REFUSE_REQUEST;
            }
            m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            QMessageBox::information(this,"添加好友",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_AGREE_RESPOND:
        {
            QMessageBox::information(this,"添加好友",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_REFUSE_RESPOND:
        {
            QMessageBox::information(this,"添加好友",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
        {
            qDebug()<<"case_respond";
            OpeWidget::getIntance().getFriend()->updateFriendList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            memcpy(caName,pdu->caData,32);
            QMessageBox::information(this,"删除好友",QString("%1 删除你作为他的好友").arg(caName));
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
        {
            QMessageBox::information(this,"删除好友","删除好友成功");
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            if(PrivateChat::getInstance().isHidden())
            {
                PrivateChat::getInstance().show();
            }
            char caSendName[32] = {'\0'};
            memcpy(caSendName,pdu->caData,32);
            QString strSendName = caSendName;
            PrivateChat::getInstance().setChatName(strSendName);
            PrivateChat::getInstance().updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            OpeWidget::getIntance().getFriend()->updateGroupMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            QMessageBox::information(this,"创建文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            OpeWidget::getIntance().getBook()->updateFileList(pdu);
            QString strEnterDir = OpeWidget::getIntance().getBook()->enterDir();
            if(!strEnterDir.isEmpty())
            {
                m_strCurPath = m_strCurPath +"/" + strEnterDir;
                qDebug()<<"enter dir:"<<m_strCurPath;
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
        {
            QMessageBox::information(this,"删除文件夹",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        {
            QMessageBox::information(this,"重命名文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            OpeWidget::getIntance().getBook()->clearEnterDir();
            QMessageBox::information(this,"进入文件夹",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_RESPOND:
        {
            QMessageBox::information(this,"删除文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {
            QMessageBox::information(this,"上传文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            qDebug()<<"数据："<<pdu->caData;
            char caFileName[32] = {'\0'};
            sscanf(pdu->caData,"%s %lld",caFileName,&(OpeWidget::getIntance().getBook()->m_iTotal));
            qDebug()<<caFileName<<"字节："<<OpeWidget::getIntance().getBook()->m_iTotal;
            if(strlen(caFileName) > 0 && OpeWidget::getIntance().getBook()->m_iTotal > 0)
            {

                OpeWidget::getIntance().getBook()->setDownloadStatus(true);
                m_file.setFileName(OpeWidget::getIntance().getBook()->getSaveFilePath());
                if(!m_file.open(QIODevice::WriteOnly))
                {
                    QMessageBox::warning(this,"下载文件","获得保存文件的路径失败");
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
        {
            QMessageBox::information(this,"共享文件",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
        {
            qDebug()<<"客户端note";
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            char *pos = strrchr(pPath,'/');
            if(NULL != pos)
            {
                pos++;
                QString strNote = QString("%1 share file->%2 \n Do you accept ?").arg(pdu->caData).arg(pos);
                int ret = QMessageBox::question(this,"共享文件",strNote);
                if(QMessageBox::Yes == ret)
                {
                    PDU *respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg,pdu->caMsg,pdu->uiMsgLen);
                    QString strName = tcpclient::getInstance().loginName();
                    strcpy(respdu->caData,strName.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
                }

            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
        {
            QMessageBox::information(this,"移动文件",pdu->caData);
            break;
        }
        default:
            break;
        }
        free(pdu);//释放数据
        pdu = NULL;
    }
    else
    {
        QByteArray buffer =  m_tcpSocket.readAll();
        m_file.write(buffer);
        Book *pBook = OpeWidget::getIntance().getBook();
        pBook->m_iRecved += buffer.size();
        if(pBook->m_iTotal == pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this,"下载文件","下载文件成功");
        }
        else if(pBook->m_iTotal < pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::critical(this,"下载文件","下载文件失败");
        }
    }
}



#if 0
void tcpclient::on_sendpb_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.toUtf8().size()+1);
        pdu->uiMsgType = 8888;
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.toUtf8().size());
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"信息发送","发送信息不能为空");
    }
}

#endif


void tcpclient::on_login_pb_clicked()//点击登录
{
    QString strName = ui->name_le->text();//获取name
    QString strPwd = ui->pwd_le->text();//获取pwd
    if(!strName.isEmpty() && !strPwd.isEmpty())//是否为空
    {
        m_strLoginName = strName;
        PDU *pdu = mkPDU(0);//新建pdu
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;//定义消息类型
        strncpy(pdu->caData,strName.toStdString().c_str(),32);//赋值name
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);//赋值pwd
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//发送数据
        qDebug()<<"发送了";
        free(pdu);
        pdu = NULL;//释放内存

    }
    else
    {
        QMessageBox::critical(this,"登录","登录失败，用户名或者密码为空");
    }

}


void tcpclient::on_regist_pb_clicked()//点击注册
{
    QString strName = ui->name_le->text();//name
    QString strPwd = ui->pwd_le->text();//pwd
    if(!strName.isEmpty() && !strPwd.isEmpty())//判空
    {
        PDU *pdu = mkPDU(0);//新建pdu
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;//定义
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
                qDebug()<<"发送了";
        free(pdu);
        pdu = NULL;

    }
    else
    {
        QMessageBox::critical(this,"注册","注册失败，用户名或者密码为空");
    }
}


void tcpclient::on_cancel_pb_clicked()
{

}

