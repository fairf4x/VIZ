#include "actions_predicateDialog.h"
#include "NodeStructure.h"
#include <iostream>

using namespace std;

Actions_predicateDialog::Actions_predicateDialog(QStringList predicates, QWidget * parent): QDialog(parent)
{
	ui.setupUi(this);
	ui.predComboBox->addItems(predicates);	
	ui.predComboBox->setFocus();
}

QString Actions_predicateDialog::predicateName()
{
	return ui.predComboBox->currentText();
}

QStringList Actions_predicateDialog::predicateSet()
{
	QStringList result;
	
	if ( ui.precondCheck->isChecked() )
		result << NSPS_PRECOND;
	
	if ( ui.effectPosCheck->isChecked() )
		result << NSPS_EFFECT_POS;

	if ( ui.effectNegCheck->isChecked() )
		result << NSPS_EFFECT_NEG;

	return result;
}

void Actions_predicateDialog::on_precondCheck_toggled(bool checked)
{
	ui.effectPosCheck->setEnabled( !checked && !ui.effectNegCheck->isChecked() );	
}

void Actions_predicateDialog::on_effectPosCheck_toggled(bool checked)
{
	ui.effectNegCheck->setEnabled(!checked);
	ui.precondCheck->setEnabled(!checked);
}

void Actions_predicateDialog::on_effectNegCheck_toggled(bool checked)
{
	ui.effectPosCheck->setEnabled( !checked && !ui.precondCheck->isChecked() );
}
