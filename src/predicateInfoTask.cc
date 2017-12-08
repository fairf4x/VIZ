#include "predicateInfoTask.h"
#include "DataWidget.h"
#include "NodeStructure.h"
#include "CheckFunctions.h"
#include "EditWidget.h"		/* kvuli definici EditWidget::ChangeCode */

using namespace XMLh;
using namespace CheckFn;

#define CLASS_COLUMN 0
#define OBJECT_COLUMN 1
#define COLUMN_COUNT 2

#define EMPTY_ARGUMENT "EMPTY"

PredicateInfoTask::PredicateInfoTask(DataWidget * data, const DataWidget * definition, int predicateID, QWidget * parent):
InfoPanel(data,predicateID,parent)
{
	ui.setupUi(this);

	xmlDefinition = definition;
	
	predicateDef = selectPredicateDefinition(definition->diagramRootElement(),data->findNode(predicateID));

	if ( verifyNodeType(selectedNode,NST_PREDICATE) )
	{
		ui.predicateName->setText(subelementTagValue(selectedNode,"label"));
		fillArgumentTable();
		setRadioButtons();
	}
}

void PredicateInfoTask::updateContent()
{
	ui.argumentsTable->clearContents();
	predicateDef = selectPredicateDefinition(xmlDefinition->diagramRootElement(),selectedNode);
	fillArgumentTable();
}

void PredicateInfoTask::on_moveDown_pressed()
{
	/* test, jestli je vubec neco vybrano */
	if (ui.argumentsTable->selectedItems().count() > 0)
	{
		moveCurrentRow(MoveDown);
	}

	emit madeChange(EditWidget::ArgumentOrderChanged);
}

void PredicateInfoTask::on_moveUp_pressed()
{
	/* test, jestli je vubec neco vybrano */
	if (ui.argumentsTable->selectedItems().count() > 0)
	{
		moveCurrentRow(MoveUp);
	}

	emit madeChange(EditWidget::ArgumentOrderChanged);
}

void PredicateInfoTask::on_initRadioButton_toggled(bool checked)
{
	if (checked)
	{
		setPredicateState(NSTS_INIT);
		xmlData->refreshNode(getIntAttribute(selectedNode,"id"));
	}
	
	emit madeChange(EditWidget::ObjectStateChanged);
}

void PredicateInfoTask::on_goalRadioButton_toggled(bool checked)
{
	if (checked)
	{
		setPredicateState(NSTS_GOAL);
		xmlData->refreshNode(getIntAttribute(selectedNode,"id"));
	}

	emit madeChange(EditWidget::ObjectStateChanged);
}

void PredicateInfoTask::fillArgumentTable()
{	 
	/* inicializuj tabulku */
	ui.argumentsTable->setColumnCount(COLUMN_COUNT);	
	ui.argumentsTable->setHorizontalHeaderLabels(QStringList() << "Argument class" << "Object");

	QDomElement connections = predicateDef.firstChildElement("connections");
	QDomElement localConn = selectedNode.firstChildElement("connections");

	QStringList verticalLabels;

	/* pro dany predikat nejsou nadefinovany zadne argumenty */
	if (connections.isNull())
		return;

	QDomNodeList argList = connections.childNodes();
	
	/* inicializace elementu - pokud je localConn.isNull() cyklus vsude prida nedefinovane hodnoty */
	QDomElement localElement = QDomElement();

	ui.argumentsTable->setRowCount(argList.count());
	for (int i=0; i < argList.count(); ++i)
	{
		QString argNum = argList.at(i).toElement().attribute("argn");
		verticalLabels.append(argNum);

		if ( !localConn.isNull() )
			localElement = findSubelementAttr(localConn,"starts","argn",argNum);	
	
		if ( !localElement.isNull() )
		{
			/* zjisti cislo hrany (v aktualne zobrazenem diagramu) */
			int locEdgeID = getIntValue(localElement);
			
			/* zjisti cisla spojovanych uzlu v aktualnim diagramu */
			QPair<int,int> nodeLoc = xmlData->associatedNodes(locEdgeID);
 		
			/* pridej do tabulky nazev promenne - pokud je pro tento argument specifikovana */
			ui.argumentsTable->setItem(i,OBJECT_COLUMN,
				new QTableWidgetItem(xmlData->getNodeData(nodeLoc.second,"label"),QTableWidgetItem::Type));
		}
		else
		{
			/* v editovanem diagramu zatim neni na tuto pozici pripojena hrana */
			ui.argumentsTable->setItem(i,OBJECT_COLUMN,
				new QTableWidgetItem(EMPTY_ARGUMENT,QTableWidgetItem::Type));
		}

		/* zjisti cislo pripojene hrany v definici */
		int defEdgeID = getIntValue(argList.at(i).toElement());
		
		/* zjisti cisla uzlu, ktere spojuje */
		QPair<int,int> nodeDef = xmlDefinition->associatedNodes(defEdgeID);
		
		QTableWidgetItem * newItem = new QTableWidgetItem(xmlDefinition->getNodeData(nodeDef.second,"label"),
									QTableWidgetItem::Type);
		newItem->setFlags(Qt::ItemIsEnabled);
		newItem->setBackground(QBrush(Qt::lightGray));

		/* pridej do tabulky jmeno pripojene tridy */ 	
		ui.argumentsTable->setItem(i,CLASS_COLUMN,newItem);
	}

	ui.argumentsTable->setVerticalHeaderLabels(verticalLabels);
}

void PredicateInfoTask::setRadioButtons()
{
	QDomElement state = selectedNode.firstChildElement("state");

	Q_ASSERT(!state.isNull());

	if ( getStrValue(state) == NSTS_INIT )
	{
		ui.initRadioButton->setChecked(true);
		return;
	}

	if ( getStrValue(state) == NSTS_GOAL )
	{
		ui.goalRadioButton->setChecked(true);
		return;
	}
	
	/* nerozpoznany stav */
	Q_ASSERT(false);
}

void PredicateInfoTask::moveCurrentRow(RowMoveDirection direction)
{
	QTableWidgetItem * current = ui.argumentsTable->currentItem();

	/* presouvat lze pouze hodnoty ve sloupci objektu */
	if (current->column() != OBJECT_COLUMN)
		return;

	int targetIndex;
	switch (direction)
	{
		case MoveUp:
			/* test na prvni radkek */
			if ( ((targetIndex = current->row() - 1) >= 0) )
			{
				swapConnections(current->row(),targetIndex);
				swapTableItems(current,ui.argumentsTable->item(targetIndex,OBJECT_COLUMN));
			}
			
		break;
		case MoveDown:
			/* test na posledni radek */
			if ( ((targetIndex = current->row() + 1) < ui.argumentsTable->rowCount()) )
			{
				swapConnections(current->row(),targetIndex);
				swapTableItems(current,ui.argumentsTable->item(targetIndex,OBJECT_COLUMN));
			}
		break;
	}
}

void PredicateInfoTask::swapTableItems(QTableWidgetItem * first, QTableWidgetItem * second)
{
	QString temp = first->text();
	first->setText(second->text());
	second->setText(temp);
	ui.argumentsTable->setCurrentItem(second);
}

void PredicateInfoTask::swapConnections(int firstArg, int secondArg)
{
	QDomElement connections = selectedNode.firstChildElement("connections");

	QString firstArgNum;
	QString secondArgNum;

	/* je potreba pricist 1 protoze radky v ui.argumentsTable se indexuji od 0
	 * a argumenty v XML reprezentaci od 1 */
	firstArgNum.setNum(firstArg+1,10);
	secondArgNum.setNum(secondArg+1,10);

	QDomElement firstConn = findSubelementAttr(connections,"starts","argn",firstArgNum);
	QDomElement secondConn = findSubelementAttr(connections,"starts","argn",secondArgNum);

	if ( secondConn.isNull() )
	{
		/* pokud cilovy argument chybi, pouze zmen cislo prvniho */
		firstConn.setAttribute("argn",secondArg);
	}
	else
	{
		int firstValue = getIntValue(firstConn);
		int secondValue = getIntValue(secondConn);
		
		setIntValue(firstConn,secondValue);
		setIntValue(secondConn,firstValue);
	}
}

void PredicateInfoTask::setPredicateState(QString newState)
{
	QDomElement stateEl = selectedNode.firstChildElement("state");

	/* pokud neni pritomen element obsahujici stav, tak ho pridej */
	if ( stateEl.isNull() )
	{
		QDomDocument owner = selectedNode.ownerDocument();
		stateEl = owner.createElement("state");
		selectedNode.appendChild(stateEl);
	}

	setStrValue(stateEl,newState);	
}
