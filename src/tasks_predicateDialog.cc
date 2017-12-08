#include "tasks_predicateDialog.h"
#include "NodeStructure.h"
#include <iostream>

using namespace std;

Tasks_predicateDialog::Tasks_predicateDialog(QStringList predicates, bool radioInit, QWidget * parent): QDialog(parent)
{
	ui.setupUi(this);
	ui.predComboBox->addItems(predicates);
	ui.initButton->setChecked(radioInit);
	ui.goalButton->setChecked(!radioInit);
	ui.predComboBox->setFocus();
}

QString Tasks_predicateDialog::predicateName()
{
	return ui.predComboBox->currentText();
}

QString Tasks_predicateDialog::taskState()
{
	if (ui.initButton->isChecked())
		return NSTS_INIT;
	else
		return NSTS_GOAL;
}
