/********************************************************************************
** Form generated from reading UI file 'tcpclient.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCPCLIENT_H
#define UI_TCPCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_tcpclient
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *name_lab;
    QLineEdit *name_le;
    QHBoxLayout *horizontalLayout_3;
    QLabel *pwd_lab;
    QLineEdit *pwd_le;
    QPushButton *login_pb;
    QHBoxLayout *horizontalLayout;
    QPushButton *regist_pb;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancel_pb;

    void setupUi(QWidget *tcpclient)
    {
        if (tcpclient->objectName().isEmpty())
            tcpclient->setObjectName(QString::fromUtf8("tcpclient"));
        tcpclient->resize(506, 460);
        widget = new QWidget(tcpclient);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(40, 60, 410, 224));
        verticalLayout_2 = new QVBoxLayout(widget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        name_lab = new QLabel(widget);
        name_lab->setObjectName(QString::fromUtf8("name_lab"));
        QFont font;
        font.setPointSize(20);
        font.setBold(true);
        name_lab->setFont(font);
        name_lab->setTextInteractionFlags(Qt::TextInteractionFlag::LinksAccessibleByMouse|Qt::TextInteractionFlag::TextEditable|Qt::TextInteractionFlag::TextEditorInteraction|Qt::TextInteractionFlag::TextSelectableByKeyboard|Qt::TextInteractionFlag::TextSelectableByMouse);

        horizontalLayout_2->addWidget(name_lab);

        name_le = new QLineEdit(widget);
        name_le->setObjectName(QString::fromUtf8("name_le"));
        name_le->setFont(font);

        horizontalLayout_2->addWidget(name_le);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        pwd_lab = new QLabel(widget);
        pwd_lab->setObjectName(QString::fromUtf8("pwd_lab"));
        pwd_lab->setFont(font);

        horizontalLayout_3->addWidget(pwd_lab);

        pwd_le = new QLineEdit(widget);
        pwd_le->setObjectName(QString::fromUtf8("pwd_le"));
        pwd_le->setFont(font);
        pwd_le->setEchoMode(QLineEdit::EchoMode::Password);

        horizontalLayout_3->addWidget(pwd_le);


        verticalLayout->addLayout(horizontalLayout_3);

        login_pb = new QPushButton(widget);
        login_pb->setObjectName(QString::fromUtf8("login_pb"));
        QFont font1;
        font1.setPointSize(24);
        font1.setBold(true);
        login_pb->setFont(font1);

        verticalLayout->addWidget(login_pb);


        verticalLayout_2->addLayout(verticalLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        regist_pb = new QPushButton(widget);
        regist_pb->setObjectName(QString::fromUtf8("regist_pb"));

        horizontalLayout->addWidget(regist_pb);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancel_pb = new QPushButton(widget);
        cancel_pb->setObjectName(QString::fromUtf8("cancel_pb"));

        horizontalLayout->addWidget(cancel_pb);


        verticalLayout_2->addLayout(horizontalLayout);


        retranslateUi(tcpclient);

        QMetaObject::connectSlotsByName(tcpclient);
    } // setupUi

    void retranslateUi(QWidget *tcpclient)
    {
        tcpclient->setWindowTitle(QCoreApplication::translate("tcpclient", "tcpclient", nullptr));
        name_lab->setText(QCoreApplication::translate("tcpclient", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        pwd_lab->setText(QCoreApplication::translate("tcpclient", "\345\257\206    \347\240\201\357\274\232", nullptr));
        login_pb->setText(QCoreApplication::translate("tcpclient", "\347\231\273\345\275\225", nullptr));
        regist_pb->setText(QCoreApplication::translate("tcpclient", "\346\263\250\345\206\214", nullptr));
        cancel_pb->setText(QCoreApplication::translate("tcpclient", "\346\263\250\351\224\200", nullptr));
    } // retranslateUi

};

namespace Ui {
    class tcpclient: public Ui_tcpclient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPCLIENT_H
