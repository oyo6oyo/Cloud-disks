#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "opewidget.h"
#include "sharefile.h"

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_strEnterDir.clear();//清除进入文件夹
    m_bDownload = false;//设置为非下载状态
    m_pTimer = new QTimer;

    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名");
    m_pFlushFilePB = new QPushButton("刷新文件夹");

    QVBoxLayout *pDirVBL= new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUploadPB = new QPushButton("上传文件");
    m_pDownloadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("共享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);

    QVBoxLayout *pFileVBL= new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownloadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    connect(m_pCreateDirPB,SIGNAL(clicked(bool)),this,SLOT(createDir()));//点击创建文件夹处理
    connect(m_pFlushFilePB,SIGNAL(clicked(bool)),this,SLOT(flushFile()));//点击刷新文件夹处理
    connect(m_pDelDirPB,SIGNAL(clicked(bool)),this,SLOT(delDir()));//点击删除文件夹处理
    connect(m_pRenamePB,SIGNAL(clicked(bool)),this,SLOT(renameFile()));//点击重命名处理
    connect(m_pBookListW,SIGNAL(doubleClicked(QModelIndex)),SLOT(enterDir(QModelIndex)));//点击双击进入文件夹处理
    connect(m_pReturnPB,SIGNAL(clicked(bool)),this,SLOT(returnPre()));//点击返回处理
    connect(m_pDelFilePB,SIGNAL(clicked(bool)),this,SLOT(delRegFile()));//点击删除文件处理
    connect(m_pUploadPB,SIGNAL(clicked(bool)),this,SLOT(uploadFile()));//点击上传处理
    connect(m_pTimer,SIGNAL(timeout()),this,SLOT(uploadFileData()));//定时器处理
    connect(m_pDownloadPB,SIGNAL(clicked(bool)),this,SLOT(downloadFile()));//点击下载处理
    connect(m_pShareFilePB,SIGNAL(clicked(bool)),this,SLOT(shareFile()));//点击分享文件处理
    connect(m_pMoveFilePB,SIGNAL(clicked(bool)),this,SLOT(moveFile()));//点击移动文件处理
    connect(m_pSelectDirPB,SIGNAL(clicked(bool)),this,SLOT(selectDestDir()));//点击选择文件夹处理

}

void Book::updateFileList(const PDU *pdu)//更新文件列表
{
    if(NULL == pdu)
    {
        return;
    }
    int row = m_pBookListW->count();//列表计数
    QListWidgetItem *pItemTmp = NULL;
    while(m_pBookListW->count()>0)//刷新文件时删除旧的文件
    {
        pItemTmp = m_pBookListW->item(row-1);
        m_pBookListW->removeItemWidget(pItemTmp);
        delete pItemTmp;
        row = row - 1;
    }

    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);//更新文件条目
    for(int i = 0;i < iCount;i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;
        qDebug()<<pFileInfo->caFileName<<pFileInfo->iFileType;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(0 == pFileInfo->iFileType)//文件夹
        {
            pItem->setIcon(QIcon(QPixmap(":/dir.png")));
        }
        else if(1 == pFileInfo->iFileType)//文件
        {
            pItem->setIcon(QIcon(QPixmap(":/file.jpg")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);//添加条目
    }
}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

QString Book::enterDir()
{
    return m_strEnterDir;
}

void Book::createDir()//创建文件夹
{
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","新文件夹名字");
    if(!strNewDir.isEmpty())
    {
        if(strNewDir.toUtf8().size()>32)
        {
            QMessageBox::warning(this,"新建文件夹","新文件夹名字不能超过32字符");
        }
        else{
            QString strName = tcpclient::getInstance().loginName();//登录名
            QString strCurPath = tcpclient::getInstance().curPath();//当前路径
            PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData,strName.toStdString().c_str(),strName.toUtf8().size());//存入数据
            strncpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.toUtf8().size());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.toUtf8().size());
            tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送消息
            free(pdu);//内存释放
            pdu = NULL;
        }
    }
    else
    {
        QMessageBox::warning(this,"新建文件夹","新文件夹名字不能为空");
    }

}

void Book::flushFile()//刷新文件
{
    QString strCurPath = tcpclient::getInstance().curPath();//获得当前路径
    PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.toUtf8().size());
    tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送当前路径
    free(pdu);
    pdu = NULL;
    clearEnterDir();
}

void Book::delDir()//删除文件夹
{
    qDebug()<<"客户端";
    QString strCurPath = tcpclient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();//获取当前选中列表条目
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"删除文件","请选择要删除的文件");
    }
    else
    {
        QString strDelName = pItem->text();//获取当前条目信息
        PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.toUtf8().size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.toUtf8().size());
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//向服务端发送消息
        qDebug()<<"没毛病"<<pdu->caMsg<<pdu->caData;
        free(pdu);
        pdu = NULL;
    }
}

void Book::renameFile()//重命名文件
{
    QString strCurPath = tcpclient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"重命名文件","请选择要重命名的文件");
    }
    else
    {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this,"重命名文件","请输入新的文件名");
        if(!strNewName.isEmpty())
        {
            PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData,strOldName.toStdString().c_str(),strOldName.toUtf8().size());//存入要更改的文件名
            strncpy(pdu->caData+32,strNewName.toStdString().c_str(),strNewName.toUtf8().size());//新文件名
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.toUtf8().size());
            tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            qDebug()<<"没毛病"<<pdu->caMsg<<pdu->caData;
            free(pdu);
            pdu = NULL;
        }
        else
        {
            QMessageBox::warning(this,"重命名文件","新文件名不能为空");
        }
    }
}

void Book::enterDir(const QModelIndex &index)//进入文件夹
{
    QString strDirName = index.data().toString();
    m_strEnterDir = strDirName;
    qDebug()<<strDirName;
    QString strCurPath = tcpclient::getInstance().curPath();//获取当前路径
    PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData,strDirName.toStdString().c_str(),strDirName.toUtf8().size());
    memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.toUtf8().size());
    tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    qDebug()<<"没毛病"<<pdu->caMsg<<pdu->caData;
    free(pdu);
    pdu = NULL;
}

void Book::returnPre()//返回上一级目录
{
    QString  strCurPath = tcpclient::getInstance().curPath();//获取当前目录
    QString strRootPath = "../../StoreDb/" + tcpclient::getInstance().loginName();//获取登录名
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this,"返回失败","已经在最开始的文件夹目录中了");
    }
    else
    {
        int index = strCurPath.lastIndexOf('/');//从后向前找"/"
        strCurPath.remove(index,strCurPath.toUtf8().size() - index);//得出返回的目录
        qDebug()<<"return -->"<<strCurPath;
        tcpclient::getInstance().setCurPath(strCurPath);//设置为当前目录
        clearEnterDir();
        flushFile();//刷新当前文件夹，更新为上一级目录
    }
}

void Book::delRegFile()//删除文件
{
    QString strCurPath = tcpclient::getInstance().curPath();//当前路径
    QListWidgetItem *pItem = m_pBookListW->currentItem();//选择的列表条目
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"删除文件","请选择要删除的文件");
    }
    else
    {
        QString strDelName = pItem->text();//选择的文件名
        PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.toUtf8().size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.toUtf8().size());
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        qDebug()<<"没毛病"<<pdu->caMsg<<pdu->caData;
        free(pdu);
        pdu = NULL;
    }
}

void Book::uploadFile()//上传文件
{
    m_strUploadFilePath = QFileDialog::getOpenFileName();//选择文件框
    qDebug()<<m_strUploadFilePath;
    if(!m_strUploadFilePath.isEmpty())//选择了文件
    {
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName =m_strUploadFilePath.right(m_strUploadFilePath.toUtf8().size()-index-1);//去除文件名
        qDebug()<<strFileName;
        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();//获取文件大小
        QString strCurPath = tcpclient::getInstance().curPath();
        PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.toUtf8().size());
        sprintf(pdu->caData,"%s %lld",strFileName.toStdString().c_str(),fileSize);
        qDebug()<<"客户端传输"<<pdu->caData<<"++"<<pdu->caMsg<<"leielie"<<pdu->uiMsgType;
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

        m_pTimer->start(1000);
    }
    else
    {
        QMessageBox::warning(this,"上传文件","上传文件不能为空");
        return ;
    }

}

void Book::uploadFileData() // 上传文件数据
{
    m_pTimer->stop(); // 停止计时器
    qDebug() << "fuck"; // 输出调试信息
    QFile file(m_strUploadFilePath); // 创建一个QFile对象，用于读取要上传的文件
    if (!file.open(QIODevice::ReadOnly)) // 以只读模式打开文件，如果失败则弹出警告框
    {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }
    char *pBuffer = new char[4096]; // 分配一个大小为4096字节的缓冲区
    qint64 ret = 0; // 初始化读取文件的返回值
    while (true) // 循环读取文件内容并发送
    {
        ret = file.read(pBuffer, 4096); // 从文件中读取数据到缓冲区，最多读取4096字节
        if (ret > 0 && ret <= 4096) // 如果读取到了数据且数据量在合理范围内
        {
            tcpclient::getInstance().getTcpSocket().write(pBuffer, ret); // 将缓冲区中的数据通过TCP套接字发送出去
        }
        else if (0 == ret) // 如果读取到的数据量为0，表示已经读取完毕
        {
            break;
        }
        else // 如果读取失败，弹出警告框并退出循环
        {
            QMessageBox::warning(this, "上传文件", "上传文件失败：读文件失败");
            break;
        }
    }
    file.close(); // 关闭文件
    delete[] pBuffer; // 释放缓冲区内存
    pBuffer = NULL;
    m_strUploadFilePath.clear(); // 清空文件路径
}


void Book::downloadFile()//下载文件
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"下载文件","请选择要下载的文件");
    }
    else
    {
        QString strSaveFilePath = QFileDialog::getSaveFileName();
        if(strSaveFilePath.isEmpty())
        {
            QMessageBox::warning(this,"下载文件","请指定要保存的文件");
            m_strSaveFilePath.clear();
        }
        else
        {
            m_strSaveFilePath = strSaveFilePath;
        }
        qDebug()<<"客户端";
        QString strCurPath = tcpclient::getInstance().curPath();//获取当前路径
        PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString strFileName = pItem->text();
        strcpy(pdu->caData,strFileName.toStdString().c_str());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.toUtf8().size());
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送请求
        free(pdu);
        pdu = NULL;
    }
}

void Book::shareFile()//分享文件
{
    qDebug()<<"客户端共享：";
    QListWidgetItem *pItem = m_pBookListW->currentItem();//获取当前文件项
    if(NULL == pItem)
    {
        QMessageBox::warning(this,"分享文件","请选择要分享的文件");
        return ;
    }
    else
    {
        m_strShareFileName = pItem->text();
        qDebug() << "选中的文件为： " << m_strShareFileName;
    }
    Friend *pFriend = OpeWidget::getIntance().getFriend();//朋友实例
    QListWidget *pFriendList = pFriend->getFriendList();//好友列表
    ShareFile::getInstance().updateFriend(pFriendList);
    if(ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(pCurItem==NULL)
    {
        QMessageBox::warning(this,"移动文件","请选择要移动的文件");
    }
    else
    {
        m_strMoveFileName = pCurItem->text();
        QString strCutPath= tcpclient::getInstance().curPath();
        m_strMoveFilePath = strCutPath + '/' + m_strMoveFileName;
        m_pSelectDirPB->setEnabled(true);
    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();//获取当前选中的文件项
    if(pCurItem==NULL)
    {
        QMessageBox::warning(this,"移动文件","请选择要移动的文件");
    }
    else
    {
        QString strDestDir = pCurItem->text();
        QString strCutPath= tcpclient::getInstance().curPath();
        m_strDestDir = strCutPath + '/' + strDestDir;//文件名添加到路径后面，形成完整的文件路径
        int srcLen = m_strMoveFilePath.toUtf8().size();
        int destLen = m_strDestDir.toUtf8().size();
        PDU *pdu =mkPDU(srcLen + destLen + 2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%d %d %s",srcLen,destLen,m_strMoveFileName.toStdString().c_str());
        memcpy(pdu->caMsg,m_strMoveFilePath.toStdString().c_str(),srcLen);
        memcpy((char*)(pdu->caMsg)+(srcLen+1),m_strDestDir.toStdString().c_str(),destLen);
        tcpclient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;


    }
    m_pSelectDirPB->setEnabled(false);//启用“选择目录”按钮，以便用户可以选择一个目标目录来移动文件。

}

void Book::setDownloadStatus(bool status)
{
    m_bDownload = status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}
