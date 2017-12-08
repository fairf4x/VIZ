#include "tasks_objectDialog.h"
#include <iostream>

using namespace std;

Tasks_objectDialog::Tasks_objectDialog(QStringList classes, QWidget * parent): QDialog(parent)
{
	ui.setupUi(this);
	ui.classComboBox->addItems(classes);
	ui.name->setFocus();
}

QString Tasks_objectDialog::objectClass()
{
	return ui.classComboBox->currentText();
}

QString Tasks_objectDialog::objectName()
{
	return ui.name->text();
}
