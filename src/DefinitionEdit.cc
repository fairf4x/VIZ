#include "DefinitionEdit.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"
#include "GraphClass.h"
#include "XMLHelpFunctions.h"
#include "CheckFunctions.h"
#include "InfoPanel.h"
#include <QtDebug>
#include <igraph.h> /* definice IGRAPH_(UN)DIRECTED */

using namespace XMLh;
using namespace CheckFn;

/* widgety pro infoPanel */
#include "predicateInfoDefinition.h"
#include "classInfo.h"

DefinitionEdit::DefinitionEdit(QWidget * parent): EditWidget(parent)
{
	allowedEdgeMask = DEP_INHERITANCE | DEP_ASSOCIATION;
	allowedNodeMask = NST_CLASS | NST_PREDICATE;
}

void DefinitionEdit::defineRectangleNode(QPointF pos, int newID)
{
	/* pridavani nove tridy - <type>class</type>*/
	bool ok;
	QString newClassName = QInputDialog::getText(this, tr("Define new class"),
	tr("Class name:"), QLineEdit::Normal,
	QString(), &ok);

	/* uzivatel kliknul na Cancel */
	if ( !ok )
		return;

	/* uzivatel nezadal zadnou hodnotu */
	if ( newClassName.isEmpty() )
	{
		QMessageBox::information(this,tr("VIZ"),tr("Class name can't be empty."));
		return;
	}
	
	QStringList definedClasses = xmlData->getNodeLabelList(NST_CLASS);
	
	if ( definedClasses.contains(newClassName,Qt::CaseInsensitive) )
	{
		QMessageBox::warning(this,tr("VIZ"),tr("Class name must be unique."));
		return;
	}

	if ( newClassName.toLower() == "object" )
	{
		QMessageBox::warning(this,tr("VIZ"),tr("Name \"object\" is reserved."));
		return;
	}

	if ( !nameChecker.exactMatch(newClassName) )
	{
		QMessageBox::warning(this,tr("VIZ"),
		tr("Name has wrong format.\n- only letters, digits, \"-\" and \"_\" are allowed\n- max lenght is limited\n- must start with letter"));
		return;	
	}
	
	NodeStructure newNode;

	newNode.setData(NodeStructure::nodePosition,pos);
	newNode.setData(NodeStructure::nodeID,newID);
	newNode.setData(NodeStructure::nodeLabel,newClassName);
	newNode.setData(NodeStructure::nodeType,NST_CLASS);

	xmlData->addDataNode(newNode);	

	emit sceneChanged(RectNodeAdded);	
}

void DefinitionEdit::defineEllipseNode(QPointF pos, int newID)
{
	/* definovani noveho predikatu - <type>predicate</type> */
	bool ok;
	QString newPredicateName = QInputDialog::getText(this, tr("Define new predicate"),
	tr("Predicate name:"), QLineEdit::Normal,
	QString(), &ok);

	/* uzivatel kliknul na Cancel */
	if ( !ok )
		return;

	/* uzivatel nezadal zadnou hodnotu */
	if ( newPredicateName.isEmpty() )
	{
		QMessageBox::information(this,tr("VIZ"),tr("Predicate name can't be empty."));
		return;
	}

	if ( !nameChecker.exactMatch(newPredicateName) )
	{
		QMessageBox::warning(this,tr("VIZ"),
		tr("Name has wrong format.\n- only letters, digits, \"-\" and \"_\" are allowed\n- max lenght is limited\n- must start with letter"));
		return;	
	}

	NodeStructure newNode;

	newNode.setData(NodeStructure::nodePosition,pos);
	newNode.setData(NodeStructure::nodeID,newID);
	newNode.setData(NodeStructure::nodeLabel,newPredicateName);
	newNode.setData(NodeStructure::nodeType,NST_PREDICATE);

	xmlData->addDataNode(newNode);	

	emit sceneChanged(EllipseNodeAdded);	
}

void DefinitionEdit::renumberPredicateArguments(QDomElement toRenumber)
{
	/* od toRenumber dal (vcetne) snizi argn o 1 */
	if ( toRenumber.isNull() || (toRenumber.tagName() != "starts") )
		return;

	int value = getIntAttribute(toRenumber,"argn"); 
	value--;
	while ( !toRenumber.isNull() )
	{
		setIntAttribute(toRenumber,"argn",value);
		value ++;
		toRenumber = toRenumber.nextSiblingElement("starts");
	}
}

void DefinitionEdit::displayInfo(int nodeID)
{
	Q_ASSERT(nodeID != INVALID_ID);

	qDebug() << "$DefinitionEdit::displayInfo";
	char clickedType = xmlData->nodeType(nodeID);

	InfoPanel * infoWidget = (InfoPanel*)NULL;

	if (clickedType == NST_PREDICATE)
		infoWidget = new PredicateInfoDefinition(xmlData, nodeID);

	if (clickedType == NST_CLASS)
		infoWidget = new ClassInfo(xmlData, nodeID);

	if (infoWidget == NULL)
	{
		qWarning() << "$DefinitionEdit::displayInfo : infoWidget == NULL";
		return;
	}

	connect(infoWidget,SIGNAL(madeChange(int)),this,SLOT(handleExternChange(int)));

	emit updateInfoPanel(infoWidget); 
}

void DefinitionEdit::makeConnection(EdgeStructure & edge, int & argNum)
{
	/* definice hrany popisujici dedicne vztahy */
	if ( edge.purpose == DEP_INHERITANCE )
	{
		xmlData->addDataEdge(edge);
		xmlData->connectEdgeToNode(edge.id,true,edge.startNodeID);
		xmlData->connectEdgeToNode(edge.id,false,edge.endNodeID);
		
		emit sceneChanged(InherEdgeAdded);
		return;
	}

	Q_ASSERT(argNum != INVALID_ARGN);

	if ( edge.purpose == DEP_ASSOCIATION )
	{ 
		
		/* vsechny testy prosly - proved pripojeni a nahlas uspech */
		xmlData->addDataEdge(edge);
		QDomElement connEl = xmlData->connectEdgeToNode(edge.id,true,edge.startNodeID);
		/* ocisluj prave vytvorene spojeni */
		setIntAttribute(connEl,"argn",argNum);
		xmlData->connectEdgeToNode(edge.id,false,edge.endNodeID);

		emit sceneChanged(AssocEdgeAdded);
		return;
	}

	qCritical() << "$DefinitionEdit::makeConnection : error in edge definition !"; 
}

void DefinitionEdit::changeConnection(EdgeStructure & edge, bool isStart, int toNode, int & argNum)
{

	/* uprav XML zaznam prislusne hrany */
	xmlData->changeEdgeAssociation(edge.id,isStart,toNode);

	if ( edge.purpose == DEP_ASSOCIATION )
	{
		/* element nasledujici za odebranym - je treba precislovat argumenty */
		QDomElement toRenumber;
		
		if (isStart)	
		{
			toRenumber = xmlData->disconnectEdgeFromNode(edge.id,isStart,edge.startNodeID);
			/* precislovani argumentu v uzlu ze ktereho byl pocatek hrany odpojen
			 * (snizeni o 1) */	
			renumberPredicateArguments(toRenumber);
		}
		else
			xmlData->disconnectEdgeFromNode(edge.id,isStart,edge.endNodeID);


		QDomElement connEl = xmlData->connectEdgeToNode(edge.id,isStart,toNode);

		if (isStart)
		{
			/* ocisluj prave vytvorene spojeni */
			setIntAttribute(connEl,"argn",argNum);
		}

		emit sceneChanged(AssocEdgeReconnected);
		return;
	}

	if ( edge.purpose == DEP_INHERITANCE )
	{
		if (isStart)
			xmlData->disconnectEdgeFromNode(edge.id,isStart,edge.startNodeID);
		else
			xmlData->disconnectEdgeFromNode(edge.id,isStart,edge.endNodeID);

		xmlData->connectEdgeToNode(edge.id,isStart,toNode);

		emit sceneChanged(InherEdgeReconnected);
		return;
	}

	/* pokud se kod dostane sem, znamena to, ze hrana ma nedefinovany ucel */
	Q_ASSERT(false);
}

void DefinitionEdit::deleteConnection(int edgeID)
{
	QDomElement edge = xmlData->findEdge(edgeID);
	if ( verifyEdgePurpose(edge,DEP_ASSOCIATION) )
		emit sceneChanged(AssocEdgeDeleted);

	if ( verifyEdgePurpose(edge,DEP_INHERITANCE) )
		emit sceneChanged(InherEdgeDeleted);

	EdgeDefinition removedEdge = xmlData->delDataEdge(edgeID);
	
	/* odpojeni od prislusnych uzlu */
	QDomElement nextConnection = xmlData->disconnectEdgeFromNode(edgeID,EDGE_START,removedEdge.first);
	xmlData->disconnectEdgeFromNode(edgeID,EDGE_END,removedEdge.second);

	/* pokud byl prvni uzel predikat, je treba precislovat argumenty */
	/* !!! tim se muze rozbit semantika nadefinovanych akci a tasku !!! */
	if ( xmlData->nodeType(removedEdge.first) == NST_PREDICATE )
		renumberPredicateArguments(nextConnection);
}

bool DefinitionEdit::verifyEdge(EdgeStructure & edge, int & argNum)
{
	if ( !EditWidget::verifyEdge(edge,argNum) )
		return false;

	/* test proti vicenasobne dedicnosti trid */
	if ( edge.purpose == DEP_INHERITANCE )
	{
		/* inicializuj GraphClass */
		GraphClass * inheritanceStructure = new GraphClass(xmlData->diagramRootElement());
		inheritanceStructure->init(NST_CLASS,DEP_INHERITANCE,IGRAPH_DIRECTED);

		/* prevence vicenasobne dedicnosti */
		if ( inheritanceStructure->edgeCnt(edge.endNodeID,IGRAPH_IN) > 0 )
		{
			qWarning() << "Multiple inheritance is not allowed.";
			delete inheritanceStructure;
			return false;	
		}

		/* prevence vzniku cyklu */
		qDebug() << "$DefinitionEdit::makeConnection : startID:" << edge.startNodeID << "endID:" << edge.endNodeID;
		inheritanceStructure->addEdge(edge.startNodeID,edge.endNodeID);

		if ( inheritanceStructure->containsCycle() )
		{
			qWarning() << "Cyclic inheritance is not allowed.";
			delete inheritanceStructure;
			return false;
		}

	}

	/* urceni cisla argumentu v pripade ze se jedna o asociacni hranu */
	if ( edge.purpose == DEP_ASSOCIATION )
	{
		qDebug() << "$DefinitionEdit::verifyEdge: DEP_ASSOCIATION";
		/* zjisti pocet argumentu */
		QDomElement predicate = xmlData->findNode(edge.startNodeID);
		int argCnt = getArgCount(predicate);

		NodeStructure tempNode;
		tempNode.setData(NodeStructure::nodeType,NST_PREDICATE);
		tempNode.setData(NodeStructure::nodeLabel,subelementTagValue(predicate,"label"));

		/* najdi vsechny predikaty stejneho jmena */
		QList<QDomElement> cloneList = xmlData->selectMatchingElementList(tempNode);
		if ( cloneList.size() > 1 )
		{
			qDebug() << "$DefinitionEdit::verifyEdge: overloaded";
			/* jedna se o pretizeny predikat */
			
			/* zjisti maximalni pocet argumentu */ 
			int maxArgNum = -1;		
			foreach(QDomElement clone, cloneList)
			{
				int clArgs = getArgCount(clone);
				if (clArgs > maxArgNum)
					maxArgNum = clArgs;
			}
			
			/* pokud by se pridanim hrany prekrocil maximalni pocet
			 * argumentu u predikatu daneho jmena o vic nez 1, nic nepridavej */
			if ( (argCnt+1) > (maxArgNum+1) )
			{
				qWarning() << "All overloaded predicates must be updated at once.";
				return false;
			}

			argNum = argCnt+1;
		}
		else
		{
			qDebug() << "$DefinitionEdit::verifyEdge: not overloaded";
			/* predikat neni pretizeny */
			argNum = argCnt+1;
		}
	}

	/* vsechny testy prosly */
	return true;	
}

EditWidget::ReconnectionValue DefinitionEdit::verifyReconnection(EdgeStructure & edge, int newNodeID,
									bool startMoved, int & argNum)
{
	/* overeni cile */
	if ( newNodeID <= 0)
	{
		qWarning() << "No target for edge reconnection";
		return NoChange;
	}
	
	/* urceni ID uzlu ze ktereho se hrana odpojuje a uzlu ktery zustane pripojen */
	int fromNodeID; /* uzel ke kteremu byla hrana puvodne pripojena edgepointem kterym bylo pohnuto */
	int stableEndID; /* uzel ke kteremu hrana zustava pripojena */

	if ( startMoved )
	{
		fromNodeID = edge.startNodeID;
		stableEndID = edge.endNodeID; 
	}
	else
	{
		fromNodeID = edge.endNodeID;
		stableEndID = edge.startNodeID;
	}
	
	/* test na vznik smycky */	
	if ( stableEndID == newNodeID )
	{
		qWarning() << "Edge has to connect two different nodes";
		return NoChange;
	}

	/* neni treba menit asociaci hrany - pouze souradnice */
	if ( fromNodeID == newNodeID )
		return PosChange;

	/* zjisteni typu jednotlivych uzlu */
	char toNodeType = xmlData->nodeType(newNodeID);	/* novy uzel na ktery se ma hrana prepojit */
	char stableEndType = xmlData->nodeType(stableEndID);	/* uzel na kterem zustava pripojena */

	char newEdgePurpose;

	/* stanoveni ucelu hrany
	 * pokud prepojujeme orientovanou hranu, vyznam promenne start je nasledujici:
	 * true -> toNodeID je pocatek presunovane hrany
	 * false -> toNodeID je konec presouvane hrany */	
	if (startMoved)
		newEdgePurpose = determineEdgePurpose(toNodeType,stableEndType);
	else
		newEdgePurpose = determineEdgePurpose(stableEndType,toNodeType);

	/* pokud by se prepojenim zmenil ucel hrany */
	if ( newEdgePurpose != edge.purpose )
	{
		qWarning() << "Change of edge purpose through reconnection is not allowed";
		return NoChange;
	}
	
	if ( edge.purpose == DEP_INHERITANCE )
	{
		/* inicializuj GraphClass */
		GraphClass * inheritanceStructure = new GraphClass(xmlData->diagramRootElement());
		inheritanceStructure->init(NST_CLASS,DEP_INHERITANCE,IGRAPH_DIRECTED);

		/* prevence vicenasobne dedicnosti
		 * - staci kontrola pouze pri prepojovani koncu hran 
		 * (pri prepojeni zacatku se vicenasobna dedicnost nevytvori) */
		if ( !startMoved && inheritanceStructure->edgeCnt(newNodeID,IGRAPH_IN) > 0 )
		{
			qWarning() << "Multiple inheritance is not allowed.";
			delete inheritanceStructure;
			return NoChange;	
		}

		/* kontrola na vznik cyklu */

		/* odebrani puvodni hrany */
		qDebug() << "$verifyReconnection: start=" << edge.startNodeID << "end=" << edge.endNodeID;
		inheritanceStructure->removeEdge(edge.startNodeID,edge.endNodeID);
		
		/* pridani nove hrany - zalezi na orientaci */
		if ( startMoved )
			inheritanceStructure->addEdge(newNodeID,stableEndID);
		else
			inheritanceStructure->addEdge(stableEndID,newNodeID);

		if ( inheritanceStructure->containsCycle() )
		{
			qWarning() << "Cyclic inheritance is not allowed.";
			delete inheritanceStructure;
			return NoChange;
		}
	}
	
	/* je treba zmenit pozici a zaroven asociaci */

	if (startMoved)
	{
		/* prepojuje se pocatek hrany - je treba urcit cislo argumentu po pripojeni */
		QDomElement newStartNode = xmlData->findNode(newNodeID);
		Q_ASSERT(!newStartNode.isNull());

		/* cislo argumentu se stanovuje pouze kdyz jde o predikat
		 * (muze jit o prepojeni INHERITANCE hrany) */
		if (verifyNodeType(newStartNode,NST_PREDICATE))
			argNum = getArgCount(newStartNode) + 1;
	}

	return PosAssocChange;
}
