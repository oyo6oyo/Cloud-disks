/********************************************************************************
** Form generated from reading UI file 'tcpserver.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCPSERVER_H
#define UI_TCPSERVER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_tcpserver
{
public:

    void setupUi(QWidget *tcpserver)
    {
        if (tcpserver->objectName().isEmpty())
            tcpserver->setObjectName(QString::fromUtf8("tcpserver"));
        tcpserver->resize(800, 600);

        retranslateUi(tcpserver);

        QMetaObject::connectSlotsByName(tcpserver);
    } // setupUi

    void retranslateUi(QWidget *tcpserver)
    {
        tcpserver->setWindowTitle(QCoreApplication::translate("tcpserver", "tcpserver", nullptr));
    } // retranslateUi

};

namespace Ui {
    class tcpserver: public Ui_tcpserver {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPSERVER_H
