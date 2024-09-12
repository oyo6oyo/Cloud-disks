#include "tcpserver.h" // 引入tcpserver头文件
#include "ui_tcpserver.h" // 引入ui_tcpserver头文件
#include<QByteArray> // 引入QByteArray类
#include<QDebug> // 引入QDebug类用于调试输出
#include<QMessageBox> // 引入QMessageBox类用于显示消息框
#include<QHostAddress> // 引入QHostAddress类用于处理IP地址
#include<QFile> // 引入QFile类用于文件操作

tcpserver::tcpserver(QWidget *parent)
    : QWidget(parent) // 继承自QWidget，设置父对象为parent
    , ui(new Ui::tcpserver) // 创建ui对象并初始化
{
    ui->setupUi(this); // 调用setupUi方法设置UI界面
    loadConfig(); // 调用loadConfig方法加载配置文件
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);//监听指定ip与port
}

tcpserver::~tcpserver()
{
    delete ui; // 释放ui对象内存
}

void tcpserver::loadConfig()//加载配置文件，获取ip与端口
{
    QFile file(":/server.config"); // 创建一个QFile对象，打开名为"server.config"的资源文件
    if(file.open(QIODevice::ReadOnly)) // 如果文件成功打开
    {
        QByteArray baData = file.readAll(); // 读取文件所有内容到baData变量中
        QString strData = baData.toStdString().c_str(); // 将字节数组转换为字符串
        file.close(); // 关闭文件
        strData.replace("\n"," ");//windows是"\r\n" // 替换字符串中的换行符为空格
        QStringList strlist = strData.split(" "); // 使用空格分割字符串得到一个字符串列表
        m_strIP = strlist.at(0); // 获取第一个元素作为IP地址
        // m_strIP = "127.23.2.1";
        m_usPort = strlist.at(1).toUShort(); // 获取第二个元素并将其转换为无符号短整型作为端口号
    }
    else
    {
        QMessageBox::critical(this,"open config","open config failed"); // 如果文件打开失败，弹出错误消息框
    }
}
