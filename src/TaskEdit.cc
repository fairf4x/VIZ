#include "TaskEdit.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"
#include "XMLHelpFunctions.h"
#include "InfoPanel.h"

using namespace XMLh;

/* dialogy pro pridani uzlu */
#include "tasks_objectDialog.h"
#include "tasks_predicateDialog.h"

/* widgety pro infoPanel */
#include "predicateInfoTask.h"
#include "objectInfo.h"

TaskEdit::TaskEdit(QWidget * parent): DrivenEditWidget(parent)
{
	allowedEdgeMask = DEP_ASSOCIATION;
	allowedNodeMask = NST_OBJECT | NST_PREDICATE;

	xmlDefinition = (DataWidget*)NULL;
}

void TaskEdit::setTaskState(PredicateState state)
{
	taskState = state;
}

void TaskEdit::setPredicatesVisible(bool visible, PredicateState selector)
{
	NodeStructure tempNode;

	/* nastav filtr */
	tempNode.setData(NodeStructure::nodeType,NST_PREDICATE);
	
	switch(selector)
	{
		case InitState:
			tempNode.setData(NodeStructure::nodeState,NSTS_INIT);
		break;
		case GoalState:
			tempNode.setData(NodeStructure::nodeState,NSTS_GOAL);
		break;
		default:
			/* nerozpoznany selector */
			Q_ASSERT(false);
	}

	QList<int> predicateList = xmlData->selectMatchingIDList(tempNode);

	/* zmen viditelnost uzlu */
	foreach(int predID,predicateList)
	{
		/* skryj/odkryj uzel */
		diagram->setNodeVisible(predID,visible);		

		/* skryj/odkryj pripojene hrany */
		QList<DataEdgepoint> edgepointList = xmlData->associatedEdgepoints(predID);
		foreach(DataEdgepoint point,edgepointList)
			diagram->setEdgeVisible(point.first,visible); /* potrebujeme pouze ID hrany */
	}
}

void TaskEdit::defineRectangleNode(QPointF pos, int newID)
{
	/* pridani noveho objektu - <type>object</type> */
	/* zjisteni dostupnych trid */
	Q_ASSERT(xmlDefinition);

	QStringList availableClasses = xmlDefinition->getNodeLabelList(NST_CLASS);

	/* pokud nejsou nadefinovany zadne tridy, nelze pridavat promennou */
	if ( availableClasses.count() == 0 )
	{
		QMessageBox::information(this,tr("VIZ"),
		tr("Object class has to be defined - there are no classes defined yet."));
		return;
	}

	/* definovani nove promenne <type>variable</type> */
	Tasks_objectDialog * dialog = new Tasks_objectDialog(availableClasses,this);
	
	if ( dialog->exec() == QDialog::Accepted )
	{
		if ( dialog->objectName().isEmpty() )
		{
			QMessageBox::information(this,tr("VIZ"),tr("Object name can't be empty."));
			return;
		}

		if ( !nameChecker.exactMatch(dialog->objectName()) )
		{
			QMessageBox::warning(this,tr("VIZ"),
			tr("Name has wrong format.\n- only letters, digits, \"-\" and \"_\" are allowed\n- max lenght is limited\n- must start with letter"));
			return;	
		}

		QStringList definedObjects = xmlData->getNodeLabelList(NST_OBJECT);
		if ( definedObjects.contains(dialog->objectName(),Qt::CaseInsensitive) )
		{
			QMessageBox::warning(this,tr("VIZ"),tr("Object name must be unique in problem scope."));
			delete dialog;
			return;
		}

		NodeStructure newNode;

		newNode.setData(NodeStructure::nodePosition,pos);
		newNode.setData(NodeStructure::nodeID,newID);
		newNode.setData(NodeStructure::nodeLabel,dialog->objectName());
		newNode.setData(NodeStructure::nodeType,NST_OBJECT);
		newNode.setData(NodeStructure::nodeClass,dialog->objectClass());
		
		xmlData->addDataNode(newNode);	
	}	
	
	/* uzivatel kliknul na Cancel */	
	delete dialog;
}

void TaskEdit::defineEllipseNode(QPointF pos, int newID)
{
	/* nelze pridat predikat */
	if (taskState == NoState)
		return;
	
	Q_ASSERT(xmlDefinition);
	
	QSet<QString> availablePredicates = xmlDefinition->getValidPredicateSet();

	/* pokud nejsou nadefinovany zadne predikaty, nelze pridat novy uzel */
	if ( availablePredicates.size() == 0 )
	{
		QMessageBox::information(this,tr("VIZ"),tr("There are no predicates defined yet."));
		return;
	}
	
	/* pridani noveho predikatu */
	Tasks_predicateDialog * dialog = new Tasks_predicateDialog(availablePredicates.toList(), (taskState == InitState) , this);
	
	if ( dialog->exec() == QDialog::Accepted )
	{
		NodeStructure newNode;

		newNode.setData(NodeStructure::nodePosition,pos);
		newNode.setData(NodeStructure::nodeID,newID);
		newNode.setData(NodeStructure::nodeLabel,dialog->predicateName());
		newNode.setData(NodeStructure::nodeType,NST_PREDICATE);
		newNode.setData(NodeStructure::nodeState,dialog->taskState());
		
		xmlData->addDataNode(newNode);
	}	

	/* uzivatel kliknul na cancel */
	delete dialog;
}

void TaskEdit::displayInfo(int nodeID)
{
	Q_ASSERT(diagram);
	Q_ASSERT(xmlData);

	Q_ASSERT(nodeID != INVALID_ID);

	char clickedType = xmlData->nodeType(nodeID);

	NodeStructure tempNode;

	InfoPanel * infoWidget = (InfoPanel*)NULL;
	QList<int> matchingDefinitions;
	switch(clickedType)
	{
		case NST_PREDICATE:
			infoWidget = new PredicateInfoTask(xmlData,xmlDefinition,nodeID);
		break;
		case NST_OBJECT:
			/* v definici se hleda uzel s tagem <label>, ktery odpovida tagu <class> vybraneho uzlu */
			tempNode.setData(NodeStructure::nodeLabel,xmlData->getNodeData(nodeID,"class"));
			tempNode.setData(NodeStructure::nodeType,NST_CLASS);
			matchingDefinitions = xmlDefinition->selectMatchingIDList(tempNode);

			if ( matchingDefinitions.count() > 1 )
				qWarning() << "TaskEdit::displayInfo : too many definitions for clicked node";

			infoWidget = new ObjectInfo(xmlData,xmlDefinition,
						nodeID,matchingDefinitions.first());
		break;
		default:
			qWarning() << "$TaskEdit::displayInfo : unexpected node type";
			return;
	}
	
	connect(infoWidget,SIGNAL(madeChange(int)),this,SLOT(handleExternChange(int)));

	emit updateInfoPanel(infoWidget);	
}
