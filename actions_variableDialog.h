#ifndef ACTIONS_VARIABLE_DIALOG_H
#define ACTIONS_VARIABLE_DIALOG_H

#include <QDialog>

#include "ui_actions_variableDialog.h"

class Actions_variableDialog : public QDialog
{
	Q_OBJECT

	public:
		Actions_variableDialog(QStringList classes, QWidget * parent = 0);

		QString variableClass();
		QString variableName();

	private:

		Ui::Actions_variableDialog ui;
};

#endif
