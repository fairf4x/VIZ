#include "predicateInfoAction.h"
#include "DataWidget.h"
#include "NodeStructure.h"
#include "CheckFunctions.h"
#include "EditWidget.h"		/* kvuli definici EditWidget::ChangeCode */

using namespace XMLh;
using namespace CheckFn;

#define CLASS_COLUMN 0
#define VARIABLE_COLUMN 1
#define COLUMN_COUNT 2

#define EMPTY_ARGUMENT "EMPTY"

PredicateInfoAction::PredicateInfoAction(DataWidget * data, const DataWidget * definition, int predicateID, QWidget * parent):
InfoPanel(data,predicateID,parent)
{
	ui.setupUi(this);

	xmlDefinition = definition;

	predicateDef = selectPredicateDefinition(definition->diagramRootElement(),data->findNode(predicateID));

	if ( verifyNodeType(selectedNode,NST_PREDICATE) )
	{
		ui.predicateName->setText(subelementTagValue(selectedNode,"label"));
		fillArgumentTable();
		setCheckBoxes();
	}
}

void PredicateInfoAction::updateContent()
{
	ui.argumentsTable->clearContents();
	predicateDef = selectPredicateDefinition(xmlDefinition->diagramRootElement(),selectedNode);
	fillArgumentTable();
}

void PredicateInfoAction::on_moveDown_pressed()
{
	/* test, jestli je vubec neco vybrano */
	if (ui.argumentsTable->selectedItems().count() > 0)
	{
		moveCurrentRow(MoveDown);
	}
	
	emit madeChange(EditWidget::ArgumentOrderChanged);
}

void PredicateInfoAction::on_moveUp_pressed()
{
	/* test, jestli je vubec neco vybrano */
	if (ui.argumentsTable->selectedItems().count() > 0)
	{
		moveCurrentRow(MoveUp);
	}

	emit madeChange(EditWidget::ArgumentOrderChanged);
}

void PredicateInfoAction::on_precondCheck_toggled(bool checked)
{
	ui.effectPosCheck->setEnabled( !checked && !ui.effectNegCheck->isChecked() );

	if (checked)
		includeInSet(NSPS_PRECOND);
	else
		excludeFromSet(NSPS_PRECOND);

	xmlData->refreshNode(getIntAttribute(selectedNode,"id"));

	emit madeChange(EditWidget::PredicateSetChanged);
}

void PredicateInfoAction::on_effectPosCheck_toggled(bool checked)
{
	ui.precondCheck->setEnabled(!checked);
	ui.effectNegCheck->setEnabled(!checked);

	if (checked)
		includeInSet(NSPS_EFFECT_POS);
	else
		excludeFromSet(NSPS_EFFECT_POS);
	
	xmlData->refreshNode(getIntAttribute(selectedNode,"id"));

	emit madeChange(EditWidget::PredicateSetChanged);
}

void PredicateInfoAction::on_effectNegCheck_toggled(bool checked)
{
	ui.effectPosCheck->setEnabled( !checked && !ui.precondCheck->isChecked() );

	if (checked)
		includeInSet(NSPS_EFFECT_NEG);
	else
		excludeFromSet(NSPS_EFFECT_NEG);

	xmlData->refreshNode(getIntAttribute(selectedNode,"id"));

	emit madeChange(EditWidget::PredicateSetChanged);
}

void PredicateInfoAction::fillArgumentTable()
{	 
	/* inicializuj tabulku */
	ui.argumentsTable->setColumnCount(COLUMN_COUNT);	
	ui.argumentsTable->setHorizontalHeaderLabels(QStringList() << "Argument class" << "Variable");

	QDomElement connections = predicateDef.firstChildElement("connections");
	QDomElement localConn = selectedNode.firstChildElement("connections");
 

	QStringList verticalLabels;

	/* pokud nejsou pro predikat nadefinovany zadne argumenty neni cim plnit tabulku */
	if ( connections.isNull())
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
			ui.argumentsTable->setItem(i,VARIABLE_COLUMN,
				new QTableWidgetItem(xmlData->getNodeData(nodeLoc.second,"label"),QTableWidgetItem::Type));
		}
		else
		{
			/* v editovanem diagramu zatim neni na tuto pozici pripojena hrana */
			ui.argumentsTable->setItem(i,VARIABLE_COLUMN,
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

void PredicateInfoAction::setCheckBoxes()
{
	QDomElement setElem = selectedNode.firstChildElement("set");
	
	while ( !setElem.isNull() )
	{

		if ( getStrValue(setElem) == NSPS_EFFECT_POS )
		{
			ui.effectPosCheck->setChecked(true);
		}
		
		if ( getStrValue(setElem) == NSPS_EFFECT_NEG )
		{
			ui.effectNegCheck->setChecked(true);
		}

		if ( getStrValue(setElem) == NSPS_PRECOND )
		{
			ui.precondCheck->setChecked(true);
		}
		
		setElem = setElem.nextSiblingElement("set");
	}	
}

void PredicateInfoAction::moveCurrentRow(RowMoveDirection direction)
{
	QTableWidgetItem * current = ui.argumentsTable->currentItem();
	int targetIndex;
	switch (direction)
	{
		case MoveUp:
			/* test na prvni radkek */
			if ( ((targetIndex = current->row() - 1) >= 0) && 
				(current->column() == VARIABLE_COLUMN) )
			{
				swapConnections(current->row(),targetIndex);
				swapTableItems(current,ui.argumentsTable->item(targetIndex,VARIABLE_COLUMN));
			}
			
		break;
		case MoveDown:
			/* test na posledni radek */
			if ( ((targetIndex = current->row() + 1) < ui.argumentsTable->rowCount()) &&
				(current->column() == VARIABLE_COLUMN) )
			{
				swapConnections(current->row(),targetIndex);
				swapTableItems(current,ui.argumentsTable->item(targetIndex,VARIABLE_COLUMN));
			}
		break;
	}
}

void PredicateInfoAction::swapTableItems(QTableWidgetItem * first, QTableWidgetItem * second)
{
	QString temp = first->text();
	first->setText(second->text());
	second->setText(temp);
	ui.argumentsTable->setCurrentItem(second);
}

void PredicateInfoAction::swapConnections(int firstArg, int secondArg)
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

void PredicateInfoAction::includeInSet(QString set)
{
	QDomDocument parentDoc = selectedNode.ownerDocument();

	/* pokud uz tam je element, ktery se ma pridat */
	if ( !findSubelementVal(selectedNode,"set",set).isNull() )
		return;

	QDomElement neighbour = selectedNode.lastChildElement("set");
	
	if ( neighbour.isNull() )
		neighbour = selectedNode.firstChildElement("type");

	QDomElement newSet = parentDoc.createElement("set");
	setStrValue(newSet,set);

	selectedNode.insertAfter(newSet,neighbour);		
}

void PredicateInfoAction::excludeFromSet(QString set)
{
	QDomElement removed = findSubelementVal(selectedNode,"set",set);
	if ( removed.isNull() )
		return;

	selectedNode.removeChild(removed);
}
