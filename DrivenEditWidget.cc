#include "DrivenEditWidget.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"
#include "XMLHelpFunctions.h"
#include "CheckFunctions.h"
#include "GraphClass.h"

#define NO_POSITION (-1)

using namespace XMLh;
using namespace CheckFn;

void DrivenEditWidget::setDefinition(const DataWidget *  definition)
{
	xmlDefinition = definition;
}

void DrivenEditWidget::makeConnection(EdgeStructure & edge, int & argNum)
{
	/* pridej hranu */
	xmlData->addDataEdge(edge);

	/* pripoj zacatek */
	QDomElement connEl = xmlData->connectEdgeToNode(edge.id,EDGE_START,edge.startNodeID);

	Q_ASSERT(argNum != INVALID_ARGN);
	/* ocisluj prave vytvorene spojeni */
	setIntAttribute(connEl,"argn",argNum);

	/* pripoj konec */
	xmlData->connectEdgeToNode(edge.id,EDGE_END,edge.endNodeID);

	emit sceneChanged(EditWidget::AssocEdgeAdded);
}

void DrivenEditWidget::changeConnection(EdgeStructure & edge, bool isStart, int toNode, int & argNum)
{
	/* uprav XML zaznam prislusne hrany */
	xmlData->changeEdgeAssociation(edge.id,isStart,toNode);

	/* uprav XML zaznam stareho uzlu */
	if (isStart)
		xmlData->disconnectEdgeFromNode(edge.id,isStart,edge.startNodeID);
	else
		xmlData->disconnectEdgeFromNode(edge.id,isStart,edge.endNodeID);

	/* uprav XML zaznam noveho uzlu */
	QDomElement connEl = xmlData->connectEdgeToNode(edge.id,isStart,toNode);

	/* uprava cisla argumentu */
	if ( isStart )
	{
		Q_ASSERT(argNum != INVALID_ARGN);
		setIntAttribute(connEl,"argn",argNum);
	}

	emit sceneChanged(EditWidget::AssocEdgeReconnected);
}

void DrivenEditWidget::deleteConnection(int edgeID)
{
	EdgeDefinition removedEdge = xmlData->delDataEdge(edgeID);
	
	/* odpojeni od prislusnych uzlu */
	xmlData->disconnectEdgeFromNode(edgeID,true,removedEdge.first);
	xmlData->disconnectEdgeFromNode(edgeID,false,removedEdge.second);
}
bool DrivenEditWidget::verifyEdge(EdgeStructure & edge, int & argNum)
{
	if ( !EditWidget::verifyEdge(edge,argNum) )
		return false;

	/* prvni uzel je predikat - zajisteno ve EditWidget::verifyEdge */
	QDomElement predicate = xmlData->findNode(edge.startNodeID);

	Q_ASSERT(verifyNodeType(predicate,NST_PREDICATE));

	QDomElement argument = xmlData->findNode(edge.endNodeID);

	Q_ASSERT( verifyNodeType(argument,NST_VARIABLE) || verifyNodeType(argument,NST_OBJECT) );

	QString predicateLabel = subelementTagValue(predicate,"label");
	QString argumentClass = subelementTagValue(argument,"class");

	/* zjisti argumenty aktualne pripojene ke zkoumane instanci predikatu */
	QHash<int,QString> instArg = predicateArguments(predicate);
/*	
	qDebug() << "Connected arguments:";
	QHashIterator<int,QString> iter(instArg);
	while(iter.hasNext())
	{
		iter.next();
		qDebug() << iter.key() << ":" << iter.value();
	}
*/
	/* zjisti pozici na ktere argument definovany hranou muze byt */
	argNum = findArgPosition(predicateLabel,argumentClass,instArg);

	if ( argNum == INVALID_ARGN )
	{
		qWarning() 	<< "Node of class:" 
				<< argumentClass 
				<< "is not compatible with" << predicateLabel
				<< "in this context.";
		return false;
	}

	return true;
}

EditWidget::ReconnectionValue DrivenEditWidget::verifyReconnection(EdgeStructure & edge, int newNodeID,
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

	/* zde jsou mozne pouze asociacni hrany */
	if ( newEdgePurpose != DEP_ASSOCIATION )
	{
		qWarning() << "Only association edges are allowed.";
		return NoChange;
	}

	/* kontrola typu argumentu pred prepojenim
	 * - podle typu uzlu na ktery se hrana prepojuje probiha kontrola kompatibility argumentu
	 * - stableNode je uzel ke kteremu hrana zustava pripojena targetNode je uzel na ktery se prepojuje */
	QDomElement targetNode = xmlData->findNode(newNodeID);
	
	QDomElement stableNode;
	/* pokud se hybe startem, stabilni je konec !!*/
	if (startMoved)
		stableNode = xmlData->findNode(edge.endNodeID);
	else
		stableNode = xmlData->findNode(edge.startNodeID);

	QString predicateLabel;
	QString argumentClass;

	/* pokud je cil prepojeni predikat */
	if ( verifyNodeType(targetNode,NST_PREDICATE) )
	{
		qDebug() << "$DrivenEditWidget::verifyReconnection : reconnection target is predicate";
		predicateLabel = subelementTagValue(targetNode,"label");
		argumentClass = subelementTagValue(stableNode,"class");

		/* zjisti jake argumenty jsou pripojene ke zkoumane instanci predikatu */
		QHash<int,QString> instArg = predicateArguments(targetNode);

		/* zjisti prvni kompatibilni pozici (podle definice a s ohledem na obsazene pozice) */
		argNum = findArgPosition(predicateLabel,argumentClass,instArg);

		if ( argNum == INVALID_ARGN )
		{
			qWarning() << "Reconnection not allowed.";
			return NoChange; 
		}		
	}

	/* cil prepojeni je bud promenna nebo objekt */
	if ( verifyNodeType(targetNode,NST_VARIABLE) || verifyNodeType(targetNode,NST_OBJECT) )
	{
		predicateLabel = subelementTagValue(stableNode,"label");
		argumentClass = subelementTagValue(targetNode,"class");

		/* zjisti kolikaty argument se bude menit */
		int changedArgNum = getArgNumber(stableNode,edge.id);

		/* zjisti seznam aktualne pripojenych argumentu */
		QHash<int,QString> instArg = predicateArguments(stableNode);

		/* odebrani zmeneneho - v pripade ze se nejedna o zmenu musi byt dany argument volny */
		instArg.remove(changedArgNum);

		int sugestedArgNum = findArgPosition(predicateLabel,argumentClass,instArg);
		
		/* nebyla nalezena moznost prepojeni */
		if ( sugestedArgNum == INVALID_ARGN )
		{
			qWarning() << "Reconnection not allowed.";
			return NoChange;
		}
		
		/* nalezena moznost prepojeni se neshoduje s aktualnim cislem argumentu */
		if ( sugestedArgNum != changedArgNum )
		{
			qWarning() << "Could not verify edge reconnection - you may delete this edge and add a new one instead.";
			return NoChange;
		}
	}
	
	/* vsechny testy prosly */
	qDebug() << "$DrivenEditWidget::verifyReconnection : Vsechno proslo. argNum =" << argNum;
	return EditWidget::PosAssocChange;
}

int DrivenEditWidget::findArgPosition(const QString & predicate, const QString & argument,const QHash<int,QString> & instArg)
{
	/* zjisti seznam moznych definic */
	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeLabel,predicate);
	tempNode.setData(NodeStructure::nodeType,NST_PREDICATE);

	int result = INVALID_ARGN;

	QList<QDomElement> definitions = xmlDefinition->selectMatchingElementList(tempNode);
	if ( definitions.isEmpty() )
	{
		qWarning() << "No predicate definition available for: " << predicate;
		return result;
	}

	/* inicializace stromu pro urceni dedicnych vztahu trid */
	GraphClass * inheritanceStructure = new GraphClass(xmlDefinition->diagramRootElement());
	inheritanceStructure->init(NST_CLASS,DEP_INHERITANCE,IGRAPH_DIRECTED);

	/* zjisteni seznamu predku - vcetne tridy samotne */
	QStringList argFamily = classFamily(argument,inheritanceStructure,xmlDefinition);
	
	/* v seznamu by melo byt minimalne jmeno argumentu samotneho */
	Q_ASSERT(!argFamily.isEmpty());

	while ( !definitions.isEmpty() )
	{
		QDomElement currentDef = definitions.takeFirst();

		QHash<int,QString> defArg = predicateArguments(currentDef);

		/* porovnej argumenty v instanci predikatu a v definici */

		bool exactMatch = true;	/* predpokladame, ze se aktualni definice hodi a zkousime to poprit */

		QHashIterator<int,QString> i(instArg);
		while (i.hasNext())
		{
			i.next();

			QStringList actArgFamily = classFamily(i.value(),inheritanceStructure,xmlDefinition);
			QString defClass = defArg.value(i.key());

			/* pri prvnim nekompatibilnim argumentu ukonci cyklus*/
			if (!actArgFamily.contains(defClass))
			{
				exactMatch = false;
				break;
			}
		}

		if( !exactMatch )
			continue;	/* zkus jinou definici */

		/* vsechny argumenty, ktere jsou pripojeny ke konkretni instanci jsou i v definici 
		 * - v pripade prazdneho hashe instArg je toto pravda */

		QSet<int> defArgSet = QSet<int>::fromList(defArg.keys());
		QSet<int> instArgSet = QSet<int>::fromList(instArg.keys());

		/* mezi volnymi argumenty najdi prvni kompatibilni pozici */
		QSet<int> freePos = defArgSet - instArgSet;

		bool compatible = false;
		foreach(int pos,freePos)
		{
			QString defClass = defArg.value(pos);
			compatible = argFamily.contains(defClass);
			if(compatible)
				return pos;
		}
	}

	delete inheritanceStructure;

	return result;
}
