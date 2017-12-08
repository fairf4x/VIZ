#ifndef TASKS_PREDICATE_DIALOG_H
#define TASKS_PREDICATE_DIALOG_H

#include <QDialog>
#include <QStringList>

#include "ui_tasks_predicateDialog.h"

class Tasks_predicateDialog: public QDialog
{
	Q_OBJECT

	public:
		Tasks_predicateDialog(QStringList predicates, bool radioInit, QWidget * parent = 0);

		QString predicateName();
		QString taskState();	
	private:
		Ui::Tasks_predicateDialog ui;	
};

#endif
