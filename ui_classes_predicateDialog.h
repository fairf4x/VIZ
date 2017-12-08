/********************************************************************************
** Form generated from reading ui file 'classes_predicateDialog.ui'
**
** Created: Tue May 19 09:43:32 2009
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CLASSES_PREDICATEDIALOG_H
#define UI_CLASSES_PREDICATEDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_Classes_predicateDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *name;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Classes_predicateDialog)
    {
    if (Classes_predicateDialog->objectName().isEmpty())
        Classes_predicateDialog->setObjectName(QString::fromUtf8("Classes_predicateDialog"));
    Classes_predicateDialog->setWindowModality(Qt::ApplicationModal);
    Classes_predicateDialog->resize(184, 108);
    Classes_predicateDialog->setSizeIncrement(QSize(0, 0));
    Classes_predicateDialog->setLayoutDirection(Qt::LeftToRight);
    Classes_predicateDialog->setSizeGripEnabled(true);
    Classes_predicateDialog->setModal(true);
    gridLayout = new QGridLayout(Classes_predicateDialog);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    label = new QLabel(Classes_predicateDialog);
    label->setObjectName(QString::fromUtf8("label"));

    gridLayout->addWidget(label, 0, 0, 1, 1);

    name = new QLineEdit(Classes_predicateDialog);
    name->setObjectName(QString::fromUtf8("name"));

    gridLayout->addWidget(name, 1, 0, 1, 1);

    buttonBox = new QDialogButtonBox(Classes_predicateDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    gridLayout->addWidget(buttonBox, 2, 0, 1, 1);


    retranslateUi(Classes_predicateDialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), Classes_predicateDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), Classes_predicateDialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(Classes_predicateDialog);
    } // setupUi

    void retranslateUi(QDialog *Classes_predicateDialog)
    {
    Classes_predicateDialog->setWindowTitle(QApplication::translate("Classes_predicateDialog", "Dialog", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("Classes_predicateDialog", "Predicate name:", 0, QApplication::UnicodeUTF8));
    name->setInputMask(QApplication::translate("Classes_predicateDialog", "nnnnnnnnnnnnnnnnnnnn; ", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(Classes_predicateDialog);
    } // retranslateUi

};

namespace Ui {
    class Classes_predicateDialog: public Ui_Classes_predicateDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLASSES_PREDICATEDIALOG_H
