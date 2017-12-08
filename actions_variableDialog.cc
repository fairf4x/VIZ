#include "actions_variableDialog.h"
#include <iostream>

using namespace std;

Actions_variableDialog::Actions_variableDialog(QStringList classes, QWidget * parent): QDialog(parent)
{
	ui.setupUi(this);
	ui.classComboBox->addItems(classes);
	ui.name->setFocus();
}

QString Actions_variableDialog::variableClass()
{
	return ui.classComboBox->currentText(); 
}

QString Actions_variableDialog::variableName()
{
	return ui.name->text();
}
