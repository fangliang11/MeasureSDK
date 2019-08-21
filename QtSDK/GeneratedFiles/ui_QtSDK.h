/********************************************************************************
** Form generated from reading UI file 'QtSDK.ui'
**
** Created by: Qt User Interface Compiler version 5.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTSDK_H
#define UI_QTSDK_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtSDKClass
{
public:
    QWidget *centralWidget;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QMdiArea *mdiArea;
    QMenuBar *menuBar;
    QMenu *menu;
    QMenu *menu_2;
    QMenu *menu_3;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtSDKClass)
    {
        if (QtSDKClass->objectName().isEmpty())
            QtSDKClass->setObjectName(QStringLiteral("QtSDKClass"));
        QtSDKClass->resize(1056, 732);
        centralWidget = new QWidget(QtSDKClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(900, 80, 75, 23));
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(900, 140, 75, 23));
        mdiArea = new QMdiArea(centralWidget);
        mdiArea->setObjectName(QStringLiteral("mdiArea"));
        mdiArea->setGeometry(QRect(-1, -1, 881, 741));
        QtSDKClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtSDKClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1056, 23));
        menu = new QMenu(menuBar);
        menu->setObjectName(QStringLiteral("menu"));
        menu_2 = new QMenu(menuBar);
        menu_2->setObjectName(QStringLiteral("menu_2"));
        menu_3 = new QMenu(menuBar);
        menu_3->setObjectName(QStringLiteral("menu_3"));
        QtSDKClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtSDKClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        QtSDKClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QtSDKClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        QtSDKClass->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menuBar->addAction(menu_2->menuAction());
        menuBar->addAction(menu_3->menuAction());

        retranslateUi(QtSDKClass);
        QObject::connect(pushButton, SIGNAL(clicked()), QtSDKClass, SLOT(onButton()));
        QObject::connect(pushButton_2, SIGNAL(clicked()), QtSDKClass, SLOT(closeButton()));

        QMetaObject::connectSlotsByName(QtSDKClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtSDKClass)
    {
        QtSDKClass->setWindowTitle(QApplication::translate("QtSDKClass", "QtSDK", nullptr));
        pushButton->setText(QApplication::translate("QtSDKClass", "\346\211\223\345\274\200", nullptr));
        pushButton_2->setText(QApplication::translate("QtSDKClass", "\345\205\263\351\227\255", nullptr));
        menu->setTitle(QApplication::translate("QtSDKClass", "\346\226\207\344\273\266", nullptr));
        menu_2->setTitle(QApplication::translate("QtSDKClass", "\345\212\237\350\203\275", nullptr));
        menu_3->setTitle(QApplication::translate("QtSDKClass", "\345\205\263\344\272\216", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtSDKClass: public Ui_QtSDKClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTSDK_H
