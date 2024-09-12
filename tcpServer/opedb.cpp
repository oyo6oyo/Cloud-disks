#include "opedb.h"
#include<QMessageBox>
#include<QDebug>
OpeDB::OpeDB(QObject *parent)
    : QObject{parent} // 初始化父类
{
    m_db = QSqlDatabase::addDatabase("QSQLITE"); // 添加数据库驱动
    // init();
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance; // 单例模式，静态实例
    return instance;
}

void OpeDB::init()//打开数据库
{
    m_db.setHostName("localhost");//本地
    m_db.setDatabaseName("/home/oyoy/QtFile/tcpServer/cloud.db");
    if (m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while (query.next()) {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug()<<data;
        }
    }
    else
    {
        QMessageBox::critical(NULL,"打开数据库","打开数据库失败");
    }
}

OpeDB::~OpeDB()
{
    m_db.close(); // 关闭数据库连接
}

bool OpeDB::handleRegist(const char *name, const char *pwd)//注册处理函数
{
    if(NULL == name || NULL == pwd)//判空
    {
        qDebug()<<"name|pwd is NULL";
        return false;
    }
    //数据库处理语句
    QString data =  QString("insert into usrInfo(name,pwd) values('%1','%2');").arg(name).arg(pwd);
    QSqlQuery query;
    qDebug()<<name<<"+++"<<pwd;
    return query.exec(data);//数据库语句执行
}

bool OpeDB::handleLogin(const char *name, const char *pwd)//登录处理函数
{
    if(NULL == name || NULL == pwd)
    {
        qDebug()<<"name|pwd is NULL";
        return false;
    }
    //查看登录用户是否存在
    QString data =  QString("select * from usrInfo where name='%1' and pwd='%2' and online=0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if(query.next())//如果存在
    {
        //设置在线状态
        data =  QString("update usrInfo set online=1 where name='%1' and pwd='%2';").arg(name).arg(pwd);
        QSqlQuery query;
        query.exec(data);
        return true;
    }
    else
    {
        return false;
    }

}

void OpeDB::handleOffline(const char *name)//客户端窗口关闭处理函数
{
    if(NULL == name)
    {
        qDebug()<<"name is NULL";
        return ;
    }
    //设置离线
    QString data =  QString("update usrInfo set online=0 where name='%1';").arg(name);
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);

}

QStringList OpeDB::handleAllOnline()
{
    QString data =  QString("select name from usrInfo where online=1;");
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    while(query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if(NULL == name)
    {
        return -1;
    }
    QString data =  QString("select online from usrInfo where name='%1';").arg(name);
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        int ret = query.value(0).toInt();
        if(1 == ret)
        {
            return 1;
        }
        else if(0 == ret)
            return 0;
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(NULL == pername||NULL == name)
    {
        return -1;
    }
    QString data = QString("select * from friend where (id=(select id from usrInfo where name='%1') and friendId = (select id from usrInfo where name = '%2')) "
                           "or (id=(select id from usrInfo where name='%3') and friendId = (select id from usrInfo where name = '%4'))").arg(pername).arg(name).arg(name).arg(pername);

    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        return 0;//双方已是好友
    }
    else
    {
        data =  QString("select online from usrInfo where name='%1';").arg(pername);
        qDebug()<<data;
        QSqlQuery query;
        query.exec(data);
        if(query.next())
        {
            int ret = query.value(0).toInt();
            if(1 == ret)
            {
                return 1;//在线
            }
            else if(0 == ret)
                return 2;//不在线
        }
        else
        {
            return 3;//不存在
        }
    }
}

int OpeDB::handleAgreeAddFriend(const char *pername, const char *name)
{
    if(NULL == pername||NULL == name)
    {
        return -1;
    }
    qDebug()<<pername<<name;
    QString idData = QString("select id from usrInfo where name='%1'").arg(pername);
    QString friendIdData = QString("select id from usrInfo where name='%1'").arg(name);
    int id = 0;
    int friendId = 0;
    QSqlQuery query;
    query.exec(idData);
    if(query.next())
    {
        id = query.value(0).toInt();
    }
    query.exec(friendIdData);
    if(query.next())
    {
        friendId = query.value(0).toInt();
    }
    QString data = QString("insert into friend(id,friendId) values(%1,%2),(%3,%4);").arg(id).arg(friendId).arg(friendId).arg(id);
    QString data_Same = QString("insert into friend(id,friendId) values(%1,%2);").arg(id).arg(friendId);
    qDebug()<<data;
    if(id == friendId)
    {
        query.exec(data_Same);
    }
    else {
        query.exec(data);
    }
    return 1;
}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    qDebug()<<"handleflushfriend";
    QStringList strFriendList;
    strFriendList.clear();
    if(NULL == name)
    {
        return strFriendList;
    }
    QString data =  QString("select name from usrInfo where id in (select friendId from friend where id = (select id from usrInfo where name='%1')) and online=1;").arg(name);
    qDebug()<<data;
    strFriendList.clear();
    QSqlQuery query;
    query.exec(data);
    while(query.next())
    {
        strFriendList.append(query.value(0).toString());
        qDebug()<<"涮蜂蜡"<<query.value(0).toString();
    }
    return strFriendList;
}

bool OpeDB::handleDelFriend(const char *name, const char *friendName)
{
    if(NULL == name ||NULL == friendName)
    {
        return false;
    }
    QString data =  QString("DELETE  FROM friend WHERE id = (SELECT id FROM usrInfo WHERE  name = '%1')  AND friendid  = (SELECT id FROM usrInfo WHERE  name = '%2') or id = (SELECT id FROM usrInfo WHERE  name = '%3')  AND friendid  = (SELECT id FROM usrInfo WHERE  name = '%4');").arg(name).arg(friendName).arg(friendName).arg(name);
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);
    return true;
}
