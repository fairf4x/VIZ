#include "objectInfo.h"
#include "DataWidget.h"
#include "NodeStructure.h"
#include "EditWidget.h"		/* kvuli definici EditWidget::ChangeCode */

using namespace XMLh;

ObjectInfo::ObjectInfo(DataWidget * data, const DataWidget * definition, int objectID, int definitionID, QWidget * parent):
InfoPanel(data,objectID,parent)
{
	ui.setupUi(this);

	xmlDefinition = definition;
	objectDef = xmlDefinition->findNode(definitionID);

	ui.objectName->setText(subelementTagValue(selectedNode,"label"));
	initComboBox();
}

void ObjectInfo::on_classComboBox_currentIndexChanged(QString newClass)
{
	QDomElement classEl = selectedNode.firstChildElement("class");
	setStrValue(classEl,newClass);

	emit madeChange(EditWidget::NodeClassChanged);
}

void ObjectInfo::initComboBox()
{
	/* vyber aktualni tridy */
	QString currentClass = subelementTagValue(selectedNode,"class");

	/* naplneni daty */
	ui.classComboBox->addItems(xmlDefinition->getNodeLabelList(NST_CLASS));

	/* nastaveni aktualni tridy */
	int currentIndex = ui.classComboBox->findText(currentClass);
	ui.classComboBox->setCurrentIndex(currentIndex);
}
