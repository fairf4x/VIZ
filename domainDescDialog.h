#ifndef DOMAIN_DESC_DIALOG_H
#define DOMAIN_DESC_DIALOG_H

#include <QDialog>
#include "XMLHelpFunctions.h"
#include "ui_domainDescDialog.h"


class DomainDescDialog : public QDialog
{
	Q_OBJECT
	
	public:
		DomainDescDialog(QDomElement & domainElement, QWidget * parent = 0);
	
	private slots:

		void on_buttonBox_accepted();
	private:

		QDomElement domainEl;

		Ui::DomainDescDialog ui;
};

#endif
