#ifndef ACTIONS_PREDICATE_DIALOG_H
#define ACTIONS_PREDICATE_DIALOG_H

#include <QDialog>
#include <QStringList>

#include "ui_actions_predicateDialog.h"

class Actions_predicateDialog : public QDialog
{
	Q_OBJECT

	public:
		Actions_predicateDialog(QStringList predicates, QWidget * parent = 0);

		QString predicateName();
		QStringList predicateSet();

	private slots:

	void on_precondCheck_toggled(bool checked);
	void on_effectPosCheck_toggled(bool checked);
	void on_effectNegCheck_toggled(bool checked);

	private:
		Ui::Actions_predicateDialog ui;
};

#endif
