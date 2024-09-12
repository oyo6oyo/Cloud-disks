#include "mytcpsocket.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QDir>
#include <QFileInfoList>

MyTcpSocket::MyTcpSocket()
{
    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg())); // 连接信号槽，当有数据可读时触发recvMsg()槽函数
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffline())); // 连接信号槽，当客户端断开连接时触发clientOffline()槽函数
    m_bUpload = false; // 初始化上传标志为false
    m_pTimer = new QTimer; // 创建一个新的QTimer对象
    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(sendFileToClient())); // 连接定时器超时信号到sendFileToClient()槽函数
}

QString MyTcpSocket::getName()
{
    return m_strName; // 返回成员变量m_strName的值
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir; // 创建一个QDir对象
    dir.mkdir(strDestDir); // 在目标目录中创建一个新的目录
    dir.setPath(strSrcDir); // 设置QDir对象的路径为源目录
    QFileInfoList fileInfolist = dir.entryInfoList(); // 获取源目录下的所有文件和子目录信息
    QString srcTmp; // 临时存储源文件路径
    QString destTmp; // 临时存储目标文件路径
    for(int i=0;i<fileInfolist.size();i++) // 遍历文件信息列表
    {
        qDebug()<<"file name:"<<fileInfolist[i].fileName(); // 输出当前文件名
        if(fileInfolist[i].isFile()) // 如果当前项是文件
        {
            fileInfolist[i].fileName(); // 获取文件名（此行代码似乎多余，可以删除）
            srcTmp = strSrcDir +'/' + fileInfolist[i].fileName(); // 拼接源文件完整路径
            destTmp = strDestDir +'/' + fileInfolist[i].fileName(); // 拼接目标文件完整路径
            QFile::copy(srcTmp,destTmp); // 复制文件从源路径到目标路径
        }
        else if(fileInfolist[i].isDir()) // 如果当前项是目录
        {
            if(QString(".")==fileInfolist[i].fileName()||QString("..")==fileInfolist[i].fileName()) // 如果是当前目录或上级目录，跳过
            {
                continue;
            }
            srcTmp = strSrcDir +'/' + fileInfolist[i].fileName(); // 拼接源目录完整路径
            destTmp = strDestDir +'/' + fileInfolist[i].fileName(); // 拼接目标目录完整路径
            copyDir(srcTmp,destTmp); // 递归调用copyDir函数复制子目录
        }
    }
}


void MyTcpSocket::recvMsg()//消息响应函数
{
    if(!m_bUpload)
    {
        qDebug()<<"字节数"<<this->bytesAvailable()<<"类型"<<ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        uint  uiPDULen = 0;
        this->read((char*)&uiPDULen,sizeof(uint));//读取总大小
        uint uiMsgLen = uiPDULen - sizeof(PDU);//实际消息caMsg长度
        PDU *pdu = mkPDU(uiMsgLen);//新建对象
        this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));//读取剩余消息
        qDebug()<<"前类型"<<pdu->uiMsgType;
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);//赋值caName
        strncpy(caPwd,pdu->caData+32,32);
        qDebug()<<"类型"<<pdu->uiMsgType;
        switch(pdu->uiMsgType)//判断消息类型
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST://注册请求
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName,pdu->caData,32);//赋值caName
            strncpy(caPwd,pdu->caData+32,32);
            bool ret = OpeDB::getInstance().handleRegist(caName,caPwd);//数据库检查账户
            PDU *respdu = mkPDU(0);//生成回复消息
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;//赋值消息类型为注册响应
            if(ret)//数据库注册成功
            {
                strcpy(respdu->caData,REGIST_OK);
                QDir dir;
                qDebug()<<"create dir :"<<dir.mkdir(QString("../../StoreDb/%1").arg(caName));

            }
            else
            {
                strcpy(respdu->caData,REGIST_FAILED);
                qDebug()<<"失败";
            }
            write((char*)respdu,respdu->uiPDULen);//生成响应消息
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST://登录请求
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName,pdu->caData,32);//赋值caName
            strncpy(caPwd,pdu->caData+32,32);
            bool ret = OpeDB::getInstance().handleLogin(caName,caPwd);//数据库登录处理
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;//消息类型赋值
            if(ret)//数据库登录成功
            {
                strcpy(respdu->caData,LOGIN_OK);//赋值respdu
                m_strName = caName;
                qDebug()<<"成功";
            }
            else
            {
                strcpy(respdu->caData,LOGIN_FAILED);
                qDebug()<<"失败";
            }
            write((char*)respdu,respdu->uiPDULen);//生成响应消息
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST://在线用户请求
        {
            QStringList ret = OpeDB::getInstance().handleAllOnline();
            qDebug()<<ret.size();
            uint uiMsgLen = ret.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i = 0;i<ret.size();i++)
            {
                memcpy((char*)(respdu->caMsg)+i*32,ret.at(i).toStdString().c_str(),ret.at(i).toUtf8().size());
            }
            write((char*)respdu,respdu->uiPDULen);//生成响应消息
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST://查找用户请求
        {
            int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if(-1 == ret)
            {
                strcpy(respdu->caData,SEARCH_USR_NO);
            }
            else if(1 == ret)
            {
                strcpy(respdu->caData,SEARCH_USR_ONLINE);
            }
            else {
                strcpy(respdu->caData,SEARCH_USR_OFFLINE);
            }
            write((char*)respdu,respdu->uiPDULen);//生成响应消息
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST://添加好友请求
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName,pdu->caData+32,32);//赋值caName
            strncpy(caName,pdu->caData,32);
            qDebug()<<"添加好友请求："<<caPerName<<caName;
            int ret = OpeDB::getInstance().handleAddFriend(caPerName,caName);
            PDU *respdu = NULL;
            if(-1 == ret)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,UNKNOW_ERROR);
                write((char*)respdu,respdu->uiPDULen);//生成响应消息
                free(respdu);
                respdu = NULL;
            }
            else if(0 == ret)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,EXISTED_FRIEND);
                write((char*)respdu,respdu->uiPDULen);//生成响应消息
                free(respdu);
                respdu = NULL;
            }
            else if(1 == ret)
            {
                qDebug()<<pdu->uiMsgType;
                MyTcpServer::getInstance().resend(caPerName,caName,pdu);//类型为添加好友响应

            }
            else if(2 == ret)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
                write((char*)respdu,respdu->uiPDULen);//生成响应消息
                free(respdu);
                respdu = NULL;
            }
            else if(3 == ret)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_NO_EXIST);
                write((char*)respdu,respdu->uiPDULen);//生成响应消息
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_AGREE_REQUEST://同意添加好友请求
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName,pdu->caData,32);//赋值caName
            strncpy(caName,pdu->caData+32,32);
            qDebug()<<caPerName<<"^"<<caName;
            int ret = OpeDB::getInstance().handleAgreeAddFriend(caPerName,caName);
            PDU *respdu = NULL;
            qDebug()<<ret;
            if(ret == 1)
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_AGREE_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_AGREE);
                write((char*)respdu,respdu->uiPDULen);//生成响应消息
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_REFUSE_REQUEST://拒绝添加好友请求
        {
            PDU *respdu = NULL;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_REFUSE_RESPOND;
            strcpy(respdu->caData,ADD_FRIEND_REFUSE);
            write((char*)respdu,respdu->uiPDULen);//生成响应消息
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            qDebug()<<"到server 短case";
            char caName[32] = {'\0'};
            strncpy(caName,pdu->caData,32);//赋值caName
            QStringList ret = OpeDB::getInstance().handleFlushFriend(caName);
            uint uiMsgLen = ret.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for(int i=0;i<ret.size();i++)
            {
                memcpy((char*)(respdu->caMsg)+i*32,ret.at(i).toStdString().c_str(),ret.at(i).toUtf8().size());
            }
            write((char*)respdu,respdu->uiPDULen);//生成响应消息
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caSelfName[32] = {'\0'};
            char caFriendName[32] = {'\0'};
            strncpy(caSelfName,pdu->caData,32);//赋值caSelfName
            strncpy(caFriendName,pdu->caData+32,32);//赋值caFriendName
            OpeDB::getInstance().handleDelFriend(caSelfName,caFriendName);//处理删除好友请求
            PDU *respdu = mkPDU(0);//创建响应消息
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;//设置响应消息类型
            strcpy(respdu->caData,DEL_FRIEND_OK);//设置响应消息内容
            write((char*)respdu,respdu->uiPDULen);//发送响应消息
            free(respdu);//释放响应消息内存
            respdu = NULL;
            MyTcpServer::getInstance().resend(caFriendName,caName,pdu);//重新发送消息给好友

            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            memcpy(caName,pdu->caData,32);//复制接收到的私聊请求者名字
            memcpy(caPerName,pdu->caData+32,32);//复制接收到的私聊对象名字
            MyTcpServer::getInstance().resend(caPerName,caName,pdu);//重新发送私聊请求
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName,pdu->caData,32);//赋值caName
            QStringList onlineFriend = OpeDB::getInstance().handleFlushFriend(caName);//获取在线好友列表
            QString tmp;
            for(int i=0;i<onlineFriend.size();i++)
            {
                tmp = onlineFriend.at(i);
                MyTcpServer::getInstance().resend(tmp.toStdString().c_str(),caName,pdu);//重新发送群聊请求给在线好友
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));//获取当前路径
            bool ret =  dir.exists(strCurPath);//检查当前路径是否存在
            PDU *respdu = NULL;
            if(ret)//当前目录存在
            {
                char caNewDir[32] = {'\0'};
                memcpy(caNewDir,pdu->caData+32,32);//复制新建文件夹名称
                QString strNewPath = strCurPath + "/" + caNewDir;//拼接新路径
                qDebug()<<strNewPath;
                ret = dir.exists(strNewPath);//检查新路径是否存在
                qDebug()<<"-->>"<<ret;
                if(ret)//创建的文件已存在
                {
                    respdu = mkPDU(0);//创建响应消息
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;//设置响应消息类型
                    strcpy(respdu->caData,FILE_NAME_EXIST);//设置响应消息内容
                }
                else
                {
                    dir.mkdir(strNewPath);//创建新文件夹
                    respdu = mkPDU(0);//创建响应消息
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;//设置响应消息类型
                    strcpy(respdu->caData,CREATE_DIR_OK);//设置响应消息内容
                }
            }
            else
            {
                respdu = mkPDU(0);//创建响应消息
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;//设置响应消息类型
                strcpy(respdu->caData,DIR_NO_EXIST);//设置响应消息内容

            }
            write((char*)respdu,respdu->uiPDULen);//发送响应消息
            free(respdu);//释放响应消息内存
            respdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath = new char[pdu->uiMsgLen]; // 创建一个新的字符数组，用于存储当前路径
            memcpy(pCurPath,pdu->caMsg,pdu->uiMsgLen); // 将消息中的路径复制到新创建的字符数组中
            QDir dir(pCurPath); // 创建一个QDir对象，用于操作目录
            QFileInfoList fileInfoList = dir.entryInfoList(); // 获取目录下的所有文件信息列表
            int iFileCount = fileInfoList.size(); // 获取文件数量
            PDU *respdu = mkPDU(sizeof(FileInfo)*(iFileCount)); // 创建响应消息，大小为文件信息结构体大小乘以文件数量
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND; // 设置响应消息类型
            FileInfo *pFileInfo = NULL; // 定义一个文件信息指针
            QString strFileName; // 定义一个字符串变量，用于存储文件名
            for(int i=0;i<iFileCount;i++) // 遍历文件信息列表
            {
                pFileInfo = (FileInfo*)(respdu->caMsg)+i; // 计算当前文件信息的地址
                strFileName = fileInfoList[i].fileName(); // 获取当前文件的文件名
                memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.toUtf8().size()); // 将文件名复制到响应消息中
                if(fileInfoList[i].isDir()) // 判断是否为目录
                {
                    pFileInfo->iFileType = 0; // 如果是目录，设置文件类型为0
                }
                else if(fileInfoList[i].isFile()) // 判断是否为文件
                {
                    pFileInfo->iFileType = 1; // 如果是文件，设置文件类型为1
                }
            }
            write((char*)respdu,respdu->uiPDULen); // 发送响应消息
            free(respdu); // 释放响应消息内存
            respdu = NULL; // 将响应消息指针置空
            break;
        }

        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            char caName[32] = {'\0'}; // 创建一个字符数组用于存储目录名
            strncpy(caName,pdu->caData,32); // 将消息中的目录名复制到字符数组中
            char *pPath = new char[pdu->uiMsgLen]; // 创建一个新的字符数组，用于存储当前路径
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen); // 将消息中的路径复制到新创建的字符数组中
            QString strPath = QString("%1/%2").arg(pPath).arg(caName); // 拼接路径和目录名
            QFileInfo fileInfo(strPath); // 获取文件信息对象
            bool ret = false; // 初始化返回值
            if(fileInfo.isDir()) // 判断是否为目录
            {
                QDir dir; // 创建一个QDir对象
                dir.setPath(strPath); // 设置要删除的目录路径
                ret = dir.removeRecursively(); // 递归删除目录及其子目录和文件
            }
            else if(fileInfo.isFile()) // 判断是否为文件
            {
                ret = false; // 如果是文件，则不进行删除操作
            }
            PDU *respdu = NULL; // 初始化响应消息指针
            if(ret) // 如果删除成功
            {
                respdu = mkPDU(strlen(DEL_DIR_OK)+1); // 创建响应消息，大小为删除成功字符串长度加1
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND; // 设置响应消息类型
                memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK)); // 将删除成功的字符串复制到响应消息中
            }
            else // 如果删除失败
            {
                respdu = mkPDU(strlen(DEL_DIR_FAILURED)+1); // 创建响应消息，大小为删除失败字符串长度加1
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND; // 设置响应消息类型
                memcpy(respdu->caData,DEL_DIR_FAILURED,strlen(DEL_DIR_FAILURED)); // 将删除失败的字符串复制到响应消息中
            }
            write((char*)respdu,respdu->uiPDULen); // 发送响应消息
            free(respdu); // 释放响应消息内存
            respdu = NULL; // 将响应消息指针置空
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            char caOldName[32] = {'\0'}; // 创建一个字符数组用于存储旧文件名
            char caNewName[32] = {'\0'}; // 创建一个字符数组用于存储新文件名
            strncpy(caOldName,pdu->caData,32); // 将消息中的旧文件名复制到字符数组中
            strncpy(caNewName,pdu->caData+32,32); // 将消息中的新文件名复制到字符数组中
            char *pPath = new char[pdu->uiMsgLen]; // 创建一个新的字符数组，用于存储当前路径
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen); // 将消息中的路径复制到新创建的字符数组中
            qDebug()<<"这没错";
            QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName); // 拼接旧文件路径
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName); // 拼接新文件路径
            qDebug()<<strOldPath;
            qDebug()<<strNewPath;

            QDir dir; // 创建一个QDir对象
            bool ret= dir.rename(strOldPath,strNewPath); // 重命名文件
            PDU *respdu = mkPDU(0); // 创建响应消息，大小为0
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND; // 设置响应消息类型
            if(ret) // 如果重命名成功
            {
                strcpy(respdu->caData,RENAME_FILE_OK); // 将成功字符串复制到响应消息中
            }
            else // 如果重命名失败
            {
                strcpy(respdu->caData,RENAME_FILE_FAILUED); // 将失败字符串复制到响应消息中
            }
            write((char*)respdu,respdu->uiPDULen); // 发送响应消息
            free(respdu); // 释放响应消息内存
            respdu = NULL; // 将响应消息指针置空
            break;
        }

        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char caEnterName[32] = {'\0'}; // 创建一个字符数组用于存储目录名
            strncpy(caEnterName,pdu->caData,32); // 将消息中的目录名复制到字符数组中
            char *pPath = new char[pdu->uiMsgLen]; // 创建一个新的字符数组，用于存储当前路径
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen); // 将消息中的路径复制到新创建的字符数组中
            QString strPath = QString("%1/%2").arg(pPath).arg(caEnterName); // 拼接路径和目录名
            qDebug()<<strPath; // 输出拼接后的路径
            QFileInfo fileInfo(strPath); // 获取文件信息对象
            PDU *respdu = NULL; // 初始化响应消息指针
            if(fileInfo.isDir()) // 判断是否为目录
            {
                QDir dir(strPath); // 创建一个QDir对象
                QFileInfoList fileInfoList = dir.entryInfoList(); // 获取目录下的文件信息列表
                int iFileCount = fileInfoList.size(); // 获取文件数量
                respdu = mkPDU(sizeof(FileInfo)*(iFileCount)); // 创建响应消息，大小为文件信息结构体大小乘以文件数量
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND; // 设置响应消息类型
                FileInfo *pFileInfo = NULL; // 定义文件信息指针
                QString strFileName; // 定义字符串变量，用于存储文件名
                for(int i=0;i<iFileCount;i++) // 遍历文件信息列表
                {
                    pFileInfo = (FileInfo*)(respdu->caMsg)+i; // 计算当前文件信息的地址
                    strFileName = fileInfoList[i].fileName(); // 获取当前文件的文件名
                    memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.toUtf8().size()); // 将文件名复制到响应消息中
                    if(fileInfoList[i].isDir()) // 判断是否为目录
                    {
                        pFileInfo->iFileType = 0; // 如果是目录，设置文件类型为0
                    }
                    else if(fileInfoList[i].isFile()) // 判断是否为文件
                    {
                        pFileInfo->iFileType = 1; // 如果是文件，设置文件类型为1
                    }
                }
                write((char*)respdu,respdu->uiPDULen); // 发送响应消息
                free(respdu); // 释放响应消息内存
                respdu = NULL; // 将响应消息指针置空
            }
            else if(fileInfo.isFile()) // 判断是否为文件
            {
                respdu = mkPDU(0); // 创建响应消息，大小为0
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND; // 设置响应消息类型
                strncpy(respdu->caData,ENTER_DIR_FAILURED,strlen(ENTER_DIR_FAILURED)); // 将失败字符串复制到响应消息中
                write((char*)respdu,respdu->uiPDULen); // 发送响应消息
                free(respdu); // 释放响应消息内存
                respdu = NULL; // 将响应消息指针置空
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
        {
            char caName[32] = {'\0'}; // 创建一个字符数组用于存储文件名
            strncpy(caName,pdu->caData,32); // 将消息中的文件名复制到字符数组中
            char *pPath = new char[pdu->uiMsgLen]; // 创建一个新的字符数组，用于存储当前路径
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen); // 将消息中的路径复制到新创建的字符数组中
            QString strPath = QString("%1/%2").arg(pPath).arg(caName); // 拼接路径和文件名
            QFileInfo fileInfo(strPath); // 获取文件信息对象
            bool ret = false; // 初始化返回值
            if(fileInfo.isDir()) // 判断是否为目录
            {
                ret = false; // 如果是目录，返回false
            }
            else if(fileInfo.isFile()) // 判断是否为文件
            {
                QDir dir; // 创建一个QDir对象
                ret = dir.remove(strPath); // 删除文件，并返回操作结果
            }
            PDU *respdu = NULL; // 初始化响应消息指针
            if(ret) // 如果删除成功
            {
                respdu = mkPDU(strlen(DEL_FILE_OK)+1); // 创建响应消息，大小为删除成功字符串长度加1
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND; // 设置响应消息类型
                memcpy(respdu->caData,DEL_FILE_OK,strlen(DEL_FILE_OK)); // 将成功字符串复制到响应消息中
            }
            else // 如果删除失败
            {
                respdu = mkPDU(strlen(DEL_FILE_FAILURED)+1); // 创建响应消息，大小为删除失败字符串长度加1
                respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_RESPOND; // 设置响应消息类型
                memcpy(respdu->caData,DEL_FILE_FAILURED,strlen(DEL_FILE_FAILURED)); // 将失败字符串复制到响应消息中
            }
            write((char*)respdu,respdu->uiPDULen); // 发送响应消息
            free(respdu); // 释放响应消息内存
            respdu = NULL; // 将响应消息指针置空
            break;
        }

        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            qDebug()<<"没穿名";
            char caFileName[32] = {'\0'};
            qint64 fileSize = 0;

            sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);//赋值caName
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            m_file.setFileName(strPath);
            //以只写方式打开，文件不存在就创建文件
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
            }
            qDebug()<<strPath;
            delete []pPath;
            pPath = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            qDebug()<<"你好";
            char caFileName[32] = {'\0'};
            strcpy(caFileName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            delete []pPath;
            pPath = NULL;

            QFileInfo fileInfo(strPath);
            qint64 fileSize = fileInfo.size();
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caData,"%s %lld",caFileName,fileSize);
            write((char*)respdu,respdu->uiPDULen);
            qDebug()<<respdu->uiMsgType;
            free(respdu);
            respdu = NULL;
            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            qDebug()<<"服务端request";
            char caSendName[32] = {'\0'};
            int num = 0;
            sscanf(pdu->caData,"%s %d",caSendName,&num);
            qDebug() << "分享文件的人：" << caSendName << " 人数：" << num;
            int size = num*32;
            PDU *respdu = mkPDU(pdu->uiMsgLen - size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(respdu->caData,caSendName);
            memcpy(respdu->caMsg,(char*)(pdu->caMsg)+size,pdu->uiMsgLen-size);
            qDebug() << "接收到文件的路径为：" << respdu->caMsg;
            char caRecvName[32] = {'\0'};
            for(int i=0;i<num;i++)
            {
                memcpy(caRecvName,(char*)(pdu->caMsg)+i*32,32);
                qDebug() << "接收到文件的好友为：" << caRecvName;
                MyTcpServer::getInstance().resend(caRecvName,caName,respdu);
            }
            free(respdu);
            respdu = NULL;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData,"share file ok");
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }

        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {
            qDebug()<<"服务端note"; // 输出调试信息，表示进入服务端note处理流程
            QString strRecvPath = QString("../../StoreDb/%1").arg(pdu->caData); // 根据接收到的数据构建接收路径
            QString strShareFilePath = QString("%1").arg((char*)(pdu->caMsg)); // 根据接收到的消息构建分享文件路径
            int index = strShareFilePath.lastIndexOf('/'); // 查找最后一个斜杠的位置
            QString strFileName = strShareFilePath.right(strShareFilePath.toUtf8().size()-index-1); // 提取文件名
            qDebug() << "被分享者的路径：" << strRecvPath; // 输出调试信息，显示接收路径
            qDebug() << "被分享的文件名：" << strFileName; // 输出调试信息，显示文件名
            strRecvPath = strRecvPath + '/' + strFileName; // 拼接完整的接收路径
            qDebug() << "被分享者的路径：" << strRecvPath; // 输出调试信息，显示完整的接收路径
            qDebug() << "分享者的路径：" << strShareFilePath; // 输出调试信息，显示分享文件路径
            QFileInfo fileinfo(strShareFilePath); // 获取分享文件的信息
            if(fileinfo.isFile()) // 如果是文件
            {
                QFile::copy(strShareFilePath,strRecvPath); // 复制文件到接收路径
            }
            else if(fileinfo.isDir()) // 如果是目录
            {
                copyDir(strShareFilePath,strRecvPath); // 复制整个目录到接收路径
            }
            break; // 结束当前case分支
        }

        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST://移动文件
        {
            char caFileName[32] = {'\0'};
            int srcLen = 0;
            int destLen = 0;
            sscanf(pdu->caData, "%d %d %s", &srcLen, &destLen, caFileName);// 从pdu->caData中解析出源路径长度、目标路径长度和文件名
            char *pSrcPath = new char[srcLen + 1];
            char *pDestPath = new char[destLen + 1 + 32];
            memset(pSrcPath, '\0', srcLen + 1);
            memset(pDestPath, '\0', destLen + 1 + 32);

            memcpy(pSrcPath, pdu->caMsg, srcLen);
            memcpy(pDestPath, (char*)(pdu->caMsg) + (srcLen + 1), destLen);

            QFileInfo fileInfo(pDestPath);// 获取目标路径的文件信息
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            if(fileInfo.isDir()) // 如果目标路径是一个目录
            {
                // 在目标路径后添加文件名
                strcat(pDestPath, "/");
                strcat(pDestPath, caFileName);

                 // 尝试重命名源路径到目标路径
                bool ret = QFile::rename(pSrcPath, pDestPath);
                if(ret)
                {
                    strcpy(respdu->caData, MOVE_FILE_OK);
                }
                else
                {
                    strcpy(respdu->caData, COMMON_ERR);
                }

            }
            else if(fileInfo.isFile())// 如果目标路径是一个文件
            {
                strcpy(respdu->caData, MOVE_FILE_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        default:
            break;
            free(pdu);
            pdu = NULL;
        }

        // qDebug()<<"用户名："<<caName<<"用户密码："<<caPwd<<"消息类型："<<pdu->uiMsgType;
    }
    else//为上传时间
    {
        PDU *respdu = NULL;
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();
        if(m_iTotal == m_iRecved)//上传完毕
        {
            qDebug()<<"写玩了";
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData,UPLOAD_FILE_OK);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(m_iTotal < m_iRecved)//上传失败
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData,UPLOAD_FILE_FAILURED);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }

    }
}

void MyTcpSocket::clientOffline() // 处理客户端离线事件
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str()); // 调用数据库实例的handleOffline方法，传入客户端名称
    emit offline(this); // 发送离线信号，携带当前对象指针
}

void MyTcpSocket::sendFileToClient() // 向客户端发送文件的方法
{
    qDebug() << "来发送了"; // 输出调试信息，表示开始发送文件
    m_pTimer->stop(); // 停止计时器
    char *pData = new char[4096]; // 分配一个大小为4096字节的字符数组用于存储读取的文件数据
    qint64 ret = 0; // 定义一个变量用于存储读取文件的返回值
    while (true) // 循环读取文件并发送
    {
        qDebug() << "发送"; // 输出调试信息，表示正在发送文件
        ret = m_file.read(pData, 4096); // 从文件中读取数据到pData数组中，最多读取4096字节
        if (ret > 0 && ret <= 4096) // 如果读取到的数据大于0且小于等于4096字节
        {
            write(pData, ret); // 将读取到的数据写入套接字，发送给客户端
        }
        else if (0 == ret) // 如果读取到的数据为0，表示文件已经读取完毕
        {
            m_file.close(); // 关闭文件
            break; // 跳出循环
        }
        else if (ret < 0) // 如果读取到的数据小于0，表示发生错误
        {
            qDebug() << "发送文件给客户端过程出错"; // 输出调试信息，表示发送文件过程中出现错误
            m_file.close(); // 关闭文件
            break; // 跳出循环
        }
    }
    delete[] pData; // 释放分配的内存空间
    pData = NULL; // 将指针置空，避免野指针
}

