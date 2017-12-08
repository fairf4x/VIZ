#include "variableInfo.h"
#include "DataWidget.h"
#include "NodeStructure.h"
#include "EditWidget.h"		/* kvuli definici EditWidget::ChangeCode */

using namespace XMLh;

VariableInfo::VariableInfo(DataWidget * data, const DataWidget * definition, int variableID, int definitionID, QWidget * parent):
InfoPanel(data,variableID,parent)
{
	ui.setupUi(this);

	xmlDefinition = definition;
	variableDef = xmlDefinition->findNode(definitionID);

	ui.variableName->setText(subelementTagValue(selectedNode,"label"));
	initComboBox();
}

void VariableInfo::on_classComboBox_currentIndexChanged(QString newClass)
{
	QDomElement classEl = selectedNode.firstChildElement("class");
	setStrValue(classEl,newClass);	

	emit madeChange(EditWidget::NodeClassChanged);
}

void VariableInfo::initComboBox()
{
	/* vyber aktualni tridy */
	QString currentClass = getStrValue(selectedNode.firstChildElement("class"));

	/* naplneni daty */
	ui.classComboBox->addItems(xmlDefinition->getNodeLabelList(NST_CLASS));

	/* nastaveni aktualni tridy */
	int currentIndex = ui.classComboBox->findText(currentClass);
	ui.classComboBox->setCurrentIndex(currentIndex);
}
