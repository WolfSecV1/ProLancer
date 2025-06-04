/********************************************************************************
** Form generated from reading UI file 'PaintWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAINTWINDOW_H
#define UI_PAINTWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaintWindowClass
{
public:

    void setupUi(QWidget *PaintWindowClass)
    {
        if (PaintWindowClass->objectName().isEmpty())
            PaintWindowClass->setObjectName("PaintWindowClass");
        PaintWindowClass->resize(600, 400);

        retranslateUi(PaintWindowClass);

        QMetaObject::connectSlotsByName(PaintWindowClass);
    } // setupUi

    void retranslateUi(QWidget *PaintWindowClass)
    {
        PaintWindowClass->setWindowTitle(QCoreApplication::translate("PaintWindowClass", "PaintWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PaintWindowClass: public Ui_PaintWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAINTWINDOW_H
