#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include <QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU *pdu);//更新文件列表
    void clearEnterDir();//清除进入文件夹
    QString enterDir();//返回进入文件夹目录
    void setDownloadStatus(bool status);//设置是否下载状态
    bool getDownloadStatus();//得到下载状态
    QString getSaveFilePath();//得到保存文件路径
    QString getShareFileName();//得到分享文件名
    qint64 m_iTotal;//总的文件大小
    qint64 m_iRecved;//已收到多少


signals:
public slots://槽函数
    void createDir();//新建文件夹
    void flushFile();//刷新文件
    void delDir();//删除文件夹
    void renameFile();//重命名文件
    void enterDir(const QModelIndex &index);//进入文件夹
    void returnPre();//返回上一级目录
    void delRegFile();//删除文件
    void uploadFile();//上传文件
    void uploadFileData();//上传文件数据
    void downloadFile();//下载文件
    void shareFile();//分享文件
    void moveFile();//移动文件
    void selectDestDir();//选择移动的文件


private:
    QListWidget *m_pBookListW;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownloadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;

    QString m_strEnterDir;
    QString m_strUploadFilePath;

    QTimer *m_pTimer;
    QString m_strSaveFilePath;
    bool m_bDownload;
    QString m_strShareFileName;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;



};

#endif // BOOK_H
