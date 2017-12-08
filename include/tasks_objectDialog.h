#ifndef TASKS_OBJECT_DIALOG_H
#define TASKS_OBJECT_DIALOG_H

#include <QDialog>

#include "ui_tasks_objectDialog.h"

class Tasks_objectDialog : public QDialog
{
	Q_OBJECT

	public:
		Tasks_objectDialog(QStringList classes, QWidget * parent = 0);

		QString objectClass();
		QString objectName();

	private:

		Ui::Tasks_objectDialog ui;
};

#endif
