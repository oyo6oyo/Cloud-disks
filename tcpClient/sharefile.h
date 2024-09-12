#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QCheckBox>
#include <QListWidget>
class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);

    static ShareFile &getInstance();//设置单例

    void text();

    void updateFriend(QListWidget *pFriendList);
signals:

public slots:
    void cancelSelect();//取消选择
    void selectAll();//全选
    void okShare();//确认
    void cancelShare();//取消分享

private :
    QPushButton *m_pSelectAllPB;
    QPushButton *m_CancelSelectPB;

    QPushButton *m_pOKPB;
    QPushButton *m_CancelPB;

    QScrollArea *m_pSA;
    QWidget *m_pFriendW;
    QVBoxLayout *m_pFriendWVBL;
    QButtonGroup *m_pButtonGroup;
};

#endif // SHAREFILE_H
