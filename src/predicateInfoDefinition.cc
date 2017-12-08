#include "predicateInfoDefinition.h"
#include "DataWidget.h"
#include "NodeStructure.h"
#include "EditWidget.h"		/* kvuli definici EditWidget::ChangeCode */

using namespace XMLh;

#define CLASS_COLUMN 0
#define COLUMN_COUNT 1

PredicateInfoDefinition::PredicateInfoDefinition(DataWidget * data, int predicateID, QWidget * parent):
InfoPanel(data,predicateID,parent)
{
	ui.setupUi(this);
	
	Q_ASSERT(!selectedNode.isNull());

	if ( verifyNodeType(selectedNode,NST_PREDICATE) )
	{
		/* nastav jmeno predikatu */
		ui.predicateName->setText(subelementTagValue(selectedNode,"label")); 

		/* nastav argumanty v tabulce */
		fillArgumentTable();
	}
}

void PredicateInfoDefinition::updateContent()
{
	ui.argumentsTable->clearContents();
	fillArgumentTable();
}

void PredicateInfoDefinition::on_moveDown_pressed()
{
	/* test, jestli je vubec neco vybrano */
	if (ui.argumentsTable->selectedItems().count() > 0)
	{
		moveCurrentRow(MoveDown);
	}

	emit madeChange(EditWidget::ArgumentOrderChanged);
}

void PredicateInfoDefinition::on_moveUp_pressed()
{
	/* test, jestli je vubec neco vybrano */
	if (ui.argumentsTable->selectedItems().count() > 0)
	{
		moveCurrentRow(MoveUp);
	}

	emit madeChange(EditWidget::ArgumentOrderChanged);
}

void PredicateInfoDefinition::fillArgumentTable()
{	 
	/* inicializuj tabulku */
	ui.argumentsTable->setColumnCount(COLUMN_COUNT);
	ui.argumentsTable->setHorizontalHeaderLabels(QStringList("Argument class"));

	QDomElement connections = selectedNode.firstChildElement("connections");

	/* pokud nejsou zadna spojeni definovana */
	if (connections.isNull())
	{
		ui.argumentsTable->setRowCount(0);
		return;
	}

	QStringList verticalLabels;
	
	QDomNodeList argList = connections.childNodes();

	ui.argumentsTable->setRowCount(argList.count());
	for (int i=0; i < argList.count(); ++i)
	{
		verticalLabels.append(argList.at(i).toElement().attribute("argn"));
		
		/* zjisti cislo pripojene hrany */
		int edgeID = getIntValue(argList.at(i).toElement());
		
		/* zjisti cisla uzlu, ktere spojuje */
		QPair<int,int> nodes = xmlData->associatedNodes(edgeID);
		
		/* pridej do tabulky jmeno pripojene tridy */ 	
		ui.argumentsTable->setItem(i,CLASS_COLUMN,
				new QTableWidgetItem(xmlData->getNodeData(nodes.second,"label"),QTableWidgetItem::Type));
	}

	ui.argumentsTable->setVerticalHeaderLabels(verticalLabels);
}

void PredicateInfoDefinition::moveCurrentRow(RowMoveDirection direction)
{
	QTableWidgetItem * current = ui.argumentsTable->currentItem();
	int targetIndex;
	switch (direction)
	{
		case MoveUp:
			/* test na prvni radkek */
			if ( (targetIndex = current->row() - 1) >= 0 )
			{
				swapConnections(current->row(),targetIndex);
				swapTableItems(current,ui.argumentsTable->item(targetIndex,CLASS_COLUMN));
			}
			
		break;
		case MoveDown:
			/* test na posledni radek */
			if ( (targetIndex = current->row() + 1) < ui.argumentsTable->rowCount() )
			{
				swapConnections(current->row(),targetIndex);
				swapTableItems(current,ui.argumentsTable->item(targetIndex,CLASS_COLUMN));
			}
		break;
	}
}

void PredicateInfoDefinition::swapTableItems(QTableWidgetItem * first, QTableWidgetItem * second)
{
	QString temp = first->text();
	first->setText(second->text());
	second->setText(temp);
	ui.argumentsTable->setCurrentItem(second);
}

void PredicateInfoDefinition::swapConnections(int firstArg, int secondArg)
{
	QDomElement connections = selectedNode.firstChildElement("connections");
	Q_ASSERT(!connections.isNull());

	QString firstArgNum;
	QString secondArgNum;

	/* je potreba pricist 1 protoze radky v ui.argumentsTable se indexuji od 0
	 * a argumenty v XML reprezentaci od 1 */
	firstArgNum.setNum(firstArg+1,10);
	secondArgNum.setNum(secondArg+1,10);

	QDomElement firstConn = findSubelementAttr(connections,"starts","argn",firstArgNum);
	QDomElement secondConn = findSubelementAttr(connections,"starts","argn",secondArgNum);

	int firstValue = getIntValue(firstConn);
	int secondValue = getIntValue(secondConn);
	
	setIntValue(firstConn,secondValue);
	setIntValue(secondConn,firstValue);
}

