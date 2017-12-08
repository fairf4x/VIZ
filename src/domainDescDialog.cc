#include "domainDescDialog.h"
#include <QMessageBox>

#define MAX_DOMAIN_NAME_LENGTH (int)50

using namespace XMLh;

DomainDescDialog::DomainDescDialog(QDomElement & domainElement, QWidget * parent):
QDialog(parent)
{
	Q_ASSERT(!domainElement.isNull());
	domainEl = domainElement;

	ui.setupUi(this);

	ui.domainName->setText(domainElement.attribute("name",QString("Unnamed")));

	QDomElement description = domainElement.firstChildElement("description");

	if (!description.isNull())
		ui.domainDescription->setPlainText(description.text());
}

void DomainDescDialog::on_buttonBox_accepted()
{
	QRegExp nameChecker(QString("^([a-z]|[A-Z]|[0-9])+$"));

	/* nastaveni jmena domeny */
	QString name = ui.domainName->text();
	name.truncate(MAX_DOMAIN_NAME_LENGTH);
	if ( !name.isEmpty() && nameChecker.exactMatch(name) )
	{
		domainEl.setAttribute("name",name);
	}
	else
		QMessageBox::warning(this,tr("VIZ"),tr("Domain name not updated - wrong format.\n- only letters and digits allowed"));

	/* nastaveni popisu domeny */	
	QDomElement description = domainEl.firstChildElement("description");

	if (description.isNull())
	{
		QDomDocument parentDoc = domainEl.ownerDocument();
		description = parentDoc.createElement("description");
		domainEl.insertBefore(description,domainEl.firstChild());
	}

	setStrValue(description,ui.domainDescription->toPlainText());
}
