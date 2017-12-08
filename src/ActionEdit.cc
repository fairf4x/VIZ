#include "ActionEdit.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"
#include "XMLHelpFunctions.h"
#include "InfoPanel.h"

using namespace XMLh;

/* dialogy pro pridani uzlu */
#include "actions_variableDialog.h"
#include "actions_predicateDialog.h"

/* widgety pro infoPanel */
#include "predicateInfoAction.h"
#include "variableInfo.h"

ActionEdit::ActionEdit(QWidget * parent): DrivenEditWidget(parent)
{
	allowedEdgeMask = DEP_ASSOCIATION;
	allowedNodeMask = NST_VARIABLE | NST_PREDICATE;
	
	xmlDefinition = (DataWidget*)NULL;
}

void ActionEdit::defineRectangleNode(QPointF pos, int newID)
{
	/* pridani nove promenne - <type>variable</type>*/
	/* zjisteni dostupnych trid */
	Q_ASSERT(xmlDefinition);

	QStringList availableClasses = xmlDefinition->getNodeLabelList(NST_CLASS);

	/* pokud nejsou nadefinovany zadne tridy, nelze pridavat promennou */
	if ( availableClasses.count() == 0 )
	{
		QMessageBox::information(this,tr("VIZ"),
		tr("Variable have to belong to some class - there are no class defined yet."));
		return;
	}

	/* definovani nove promenne <type>variable</type> */
	Actions_variableDialog * dialog = new Actions_variableDialog(availableClasses,this);
	
	if ( dialog->exec() == QDialog::Accepted )
	{
		if ( dialog->variableName().isEmpty() )
		{
			QMessageBox::information(this,tr("VIZ"),tr("Variable name can't be empty."));
			return;
		}

		if ( !nameChecker.exactMatch(dialog->variableName()) )
		{
			QMessageBox::warning(this,tr("VIZ"),
			tr("Name has wrong format.\n- only letters, digits, \"-\" and \"_\" are allowed\n- max lenght is limited\n- must start with letter"));
			return;	
		}

		QStringList definedVariables = xmlData->getNodeLabelList(NST_VARIABLE);
		if ( definedVariables.contains(dialog->variableName(),Qt::CaseInsensitive) )
		{
			QMessageBox::warning(this,tr("VIZ"),tr("Variable name must be unique in action scope."));
			delete dialog;
			return;
		}
		NodeStructure newNode;

		newNode.setData(NodeStructure::nodePosition,pos);
		newNode.setData(NodeStructure::nodeID,newID);
		newNode.setData(NodeStructure::nodeLabel,dialog->variableName());
		newNode.setData(NodeStructure::nodeType,NST_VARIABLE);
		newNode.setData(NodeStructure::nodeClass,dialog->variableClass());
		
		xmlData->addDataNode(newNode);	
		changed = true;
	}	
	
	/* uzivatel kliknul na Cancel */	
	delete dialog;
}

void ActionEdit::defineEllipseNode(QPointF pos, int newID)
{
	Q_ASSERT(xmlDefinition);

	QSet<QString> availablePredicates = xmlDefinition->getValidPredicateSet();

	/* pokud nejsou nadefinovany zadne predikaty, nelze pridat novy uzel */
	if ( availablePredicates.size() == 0 )
	{
		QMessageBox::information(this,tr("VIZ"),tr("There are no predicates defined yet."));
		return;
	}
	
	/* pridani noveho predikatu */
	Actions_predicateDialog * dialog = new Actions_predicateDialog(availablePredicates.toList(),this);
	
	if ( dialog->exec() == QDialog::Accepted )
	{
		NodeStructure newNode;

		newNode.setData(NodeStructure::nodePosition,pos);
		newNode.setData(NodeStructure::nodeID,newID);
		newNode.setData(NodeStructure::nodeLabel,dialog->predicateName());
		newNode.setData(NodeStructure::nodeType,NST_PREDICATE);
		newNode.setData(NodeStructure::nodePredicateSet,dialog->predicateSet());
		
		xmlData->addDataNode(newNode);
	}	

	/* uzivatel kliknul na cancel */
	delete dialog;
}

void ActionEdit::displayInfo(int nodeID)
{
	Q_ASSERT(nodeID != INVALID_ID);

	char clickedType = xmlData->nodeType(nodeID);

	NodeStructure tempNode;
	QList<int> matchingDefinitions;
	InfoPanel * infoWidget = (InfoPanel*)NULL;
	switch(clickedType)
	{
		case NST_PREDICATE:
			infoWidget = new PredicateInfoAction(xmlData,xmlDefinition,nodeID);
		break;
		case NST_VARIABLE:
			/* v definici se hleda uzel s tagem <label>, ktery odpovida tagu <class> vybraneho uzlu */
			tempNode.setData(NodeStructure::nodeLabel,xmlData->getNodeData(nodeID,"class"));
			tempNode.setData(NodeStructure::nodeType,NST_CLASS);
			matchingDefinitions = xmlDefinition->selectMatchingIDList(tempNode);

			if ( matchingDefinitions.count() > 1 )
				qWarning() << "$ActionEdit::displayInfo : too many definitions for clicked node";
			
			infoWidget = new VariableInfo(xmlData,xmlDefinition,
						nodeID,matchingDefinitions.first());
		break;
		default:
			qWarning() << "$ActionEdit::displayInfo : wrong node type";
			return;
	}
	
	connect(infoWidget,SIGNAL(madeChange(int)),this,SLOT(handleExternChange(int)));

	emit updateInfoPanel(infoWidget);	
}

