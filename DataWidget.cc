#include "DataWidget.h"
#include "DiagramWidget.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"
#include "CheckFunctions.h"

using namespace CheckFn;

DataWidget::DataWidget(DiagramWidget * diagram)
{
	diagramScene = diagram;
	XMLdata = QDomDocument();
}

QDomElement DataWidget::diagramRootElement() const
{
	return diagramData;
}

void DataWidget::loadData(QDomDocument diagramDocument)
{
	/* vyresetovani diagramu */
	diagramScene->reset();

	/* pokud je paramatrem prazdny dokument, jedna se o reset */
	if ( diagramDocument.isNull() )
	{
		XMLdata = diagramDocument;	
		return;
	}

	/* vytvoreni kopie dokumentu (vyuziva explicitni sdileni - zmeny provedene zde se projevi i navenek) */
	XMLdata = diagramDocument;
	
	/* specifikace prave zpracovavaneho diagramu */
	diagramData = XMLdata.firstChildElement("diagram");
	
	Q_ASSERT(!diagramData.isNull());

	/* pridani hran */
	QDomElement edge = diagramData.firstChildElement("edge");
	while( !edge.isNull() )
	{
		diagramScene->addEdge(loadEdge(edge));		
		edge = edge.nextSiblingElement("edge");
	}

	/* pridani uzlu */
	QDomElement node = diagramData.firstChildElement("node");
	while( !node.isNull() )
	{
		diagramScene->addNode(loadNode(node));		
		node = node.nextSiblingElement("node");
	}
}

QList<DataEdgepoint> DataWidget::associatedEdgepoints(int nodeID) const
{
	/* nalezeni prislusneho uzlu */
	QDomElement node = findSubelementAttr(diagramData,"node","id",QString().setNum(nodeID));
	
	/* vytvoreni vysledneho seznamu */
	QList<DataEdgepoint> result;

	/* ziskani seznamu vstupujicich a vystupujicich hran */
	QDomElement connections = node.firstChildElement("connections");
	
	if (connections.isNull())
		return result;

	QDomElement edgepoint = connections.firstChild().toElement();	
	while( !edgepoint.isNull() )
	{
		if (edgepoint.tagName() == "starts")
		{
			result.append(DataEdgepoint(getIntValue(edgepoint),EDGE_START));
			edgepoint = edgepoint.nextSiblingElement();
			continue;
		}

		if (edgepoint.tagName() == "ends")
		{
			result.append(DataEdgepoint(getIntValue(edgepoint),EDGE_END));
			edgepoint = edgepoint.nextSiblingElement();
			continue;
		}
		
		qWarning() << "DataWidget::associatedEdgepoints : unexpected subelement in <connections>";
		break;
	}

	return result;
}

QList<int> DataWidget::associatedEdges(int nodeID) const
{
	QDomElement node = findSubelementAttr(diagramData,"node","id",QString().setNum(nodeID));

	return connectedEdges(node);
}

QPair<int,int> DataWidget::associatedNodes(int edgeID) const
{
	/* nalezeni prislusne hrany */
	QDomElement edge = findSubelementAttr(diagramData,"edge","id",QString().setNum(edgeID));		
	
	int startID	= associatedNodeID(edge,true);
	int endID	= associatedNodeID(edge,false); 
	
	return qMakePair(startID,endID);
}

QString DataWidget::getNodeData(int id, QString tagName) const
{
	QDomElement node = findSubelementAttr(diagramData,"node","id",QString().setNum(id));

	Q_ASSERT(!node.isNull());

	return subelementTagValue(node,tagName);	
}

const EdgeStructure DataWidget::getEdgeDescription(int id) const
{
	QDomElement edge = findSubelementAttr(diagramData,"edge","id",QString().setNum(id));
	
	return loadEdge(edge);
}

QDomElement DataWidget::findNode(int id) const
{
	return findSubelementAttr(diagramData,"node","id",QString().setNum(id));
}

QDomElement DataWidget::findEdge(int id) const
{
	return findSubelementAttr(diagramData,"edge","id",QString().setNum(id));
}

char DataWidget::readEdgePurpose(int edgeID)
{
	QDomElement edge = findSubelementAttr(diagramData,"edge","id",QString().setNum(edgeID));
	
	QDomElement purpose = edge.firstChildElement("purpose");
	
	return detEdgePurpose(purpose);
}

char DataWidget::nodeType(int nodeID)
{
	QDomElement node = findSubelementAttr(diagramData,"node","id",QString().setNum(nodeID));
	
	Q_ASSERT(!node.isNull());

	return detNodeType(node.firstChildElement("type"));
}

QSet<QString> DataWidget::getValidPredicateSet() const
{
	Q_ASSERT(!diagramData.isNull());

	QDomElement actNode = diagramData.firstChildElement("node");

	/* <label predikatu, pocet argumentu>
	 * -vysledna mapa bude obsahovat vsechny verze pretizenych predikatu */
	QMultiMap<QString,int> argNumbers;
	while ( !actNode.isNull() )
	{
		if (verifyNodeType(actNode,NST_PREDICATE))
			argNumbers.insert(subelementTagValue(actNode,"label"),getArgCount(actNode));

		actNode = actNode.nextSiblingElement("node");
	}

	QSet<QString> uniqueNames = QSet<QString>::fromList(argNumbers.uniqueKeys());
	QSet<QString> invalid;

	foreach(QString pred,uniqueNames)
	{
		/* pocty argumentu daneho predikatu */
		QSet<int> argCountSet = QSet<int>::fromList(argNumbers.values(pred));

		/* predikat je platny pouze pokud maji vsechny jeho verze stejny pocet argumentu */
		if ( argCountSet.size() != 1 )
			invalid.insert(pred);
	}

	return uniqueNames.subtract(invalid);
}

QStringList DataWidget::getNodeLabelList(char type) const
{
	QStringList result;
	
	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeType,type);

	QDomElement actNode = diagramData.firstChildElement("node");

	while ( !actNode.isNull())
	{
		if (nodeMatches(actNode, tempNode))
			result.append(getStrValue(actNode.firstChildElement("label")));	
		
		actNode = actNode.nextSiblingElement("node");
	} 

	return result;
}

QSet<QString> DataWidget::getNodeLabelSet(char type) const
{
	QSet<QString> result;
	
	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeType,type);

	Q_ASSERT(!diagramData.isNull());

	QDomElement actNode = diagramData.firstChildElement("node");

	while ( !actNode.isNull())
	{
		if (nodeMatches(actNode, tempNode))
			result.insert(getStrValue(actNode.firstChildElement("label")));	
		
		actNode = actNode.nextSiblingElement("node");
	} 

	return result;
}

int DataWidget::getMatchingNodeID(const NodeStructure & nodeTemplate) const
{
	QList<int> candidates = selectMatchingIDList(nodeTemplate);

	if ( candidates.isEmpty() )
	{
		qWarning() << "$DataWidget::getMatchingNodeID : specified node not found.";
		return INVALID_ID;
	}
	
	if ( candidates.size() > 1 )
		qWarning() << "$DataWidget::getMatchingNodeID : specified node is not unique";

	return candidates.first();
}

QList<int> DataWidget::selectMatchingIDList(const NodeStructure & nodeTemplate) const
{
	return CheckFn::selectMatchingIDList(diagramData,nodeTemplate);	
}

QList<QDomElement> DataWidget::selectMatchingElementList(const NodeStructure & nodeTemplate) const
{
	return CheckFn::selectMatchingElementList(diagramData,nodeTemplate);
}

void DataWidget::refreshNode(int nodeID)
{
	QDomElement targetNode = findSubelementAttr(diagramData,"node","id",QString().setNum(nodeID));

	diagramScene->updateNode(loadNode(targetNode));
}

void DataWidget::addDataNode(const NodeStructure & node)
{
	qDebug() << "$DataWidget::addDataNode";
	/* zatim nebyl specifikovan zadny dokument pro ukladani */
	Q_ASSERT(!XMLdata.isNull());

	QDomElement newChild = XMLdata.createElement("node");

	/* inicializace noveho uzlu */

	/* atribut id */
	setIntAttribute(newChild,"id",node.readData(NodeStructure::nodeID).toInt());
	
	/* <label> */
	addChildElement(newChild,"label",node.readData(NodeStructure::nodeLabel).toString());

	/* <type> */
	char nodeType = node.readData(NodeStructure::nodeType).toChar().toAscii();
	QStringList predSet;

	QVariant taskState = node.readData(NodeStructure::nodeState);
	
	int i;
	switch(nodeType)
	{
		case NST_PREDICATE:
			addChildElement(newChild,"type", NT_PREDICATE);
			
			/* pokud je obsazen taskState - init/goal v definici problemu */
			if ( !taskState.isNull() )
				addChildElement(newChild,"state",taskState.toString());
		
			/* pridani tagu <set> urceni mnoziny do ktere predikat nalezi */
			predSet = node.readData(NodeStructure::nodePredicateSet).toStringList();
			for (i = 0; i<predSet.size(); ++i)		
  				addChildElement(newChild,"set",predSet.at(i));
		break;
		case NST_CLASS:
			addChildElement(newChild,"type", NT_CLASS);
		break;
		case NST_OBJECT:
			addChildElement(newChild,"type", NT_OBJECT);
			addChildElement(newChild,"class", node.readData(NodeStructure::nodeClass).toString());
		break;
		case NST_VARIABLE:
			addChildElement(newChild,"type", NT_VARIABLE);
			addChildElement(newChild,"class", node.readData(NodeStructure::nodeClass).toString());
		break;
		default:
			/* nerozpoznany typ uzlu */
			Q_ASSERT(false);
	}

	/* <pos> */
	QDomElement newChildPos = XMLdata.createElement("pos");
	setPosition(newChildPos,node.readData(NodeStructure::nodePosition).toPointF());

	newChild.appendChild(newChildPos);

	/* pripojeni noveho uzlu do XML stromu */
	QDomElement lastNode = diagramData.lastChildElement("node");

	if (lastNode.isNull())
		diagramData.appendChild(newChild);
	else
		diagramData.insertAfter(newChild,lastNode);

	/* pridani noveho uzlu do DiagramWidgetu */
	diagramScene->addNode(node);
}

void DataWidget::addDataEdge(const EdgeStructure & edge)
{
	Q_ASSERT(!XMLdata.isNull());

	QDomElement newChild = XMLdata.createElement("edge");

	setIntAttribute(newChild,"id",edge.id);
	/* <purpose> */
	switch(edge.purpose)
	{
		case DEP_ASSOCIATION:
			addChildElement(newChild,"purpose",EP_ASSOCIATION);
		break;
		case DEP_INHERITANCE:
			addChildElement(newChild,"purpose",EP_INHERITANCE);
		break;
		default:
			/* nerozpoznany typ hrany */
			Q_ASSERT(false);
	}
	
	QDomElement posTag;	/* tag urcujici pozici */
	QDomElement connEl;	/* subelement uchovavajici informaci o propojeni hrany */

	/* <start> */
	connEl = XMLdata.createElement("start");
	setIntAttribute(connEl,"nid",edge.startNodeID);
	
	posTag = XMLdata.createElement("pos");
	setPosition(posTag,edge.startPos);
	connEl.appendChild(posTag);
	
	newChild.appendChild(connEl);

	/* <end> */
	connEl = XMLdata.createElement("end");
	setIntAttribute(connEl,"nid",edge.endNodeID);
	
	posTag = XMLdata.createElement("pos");
	setPosition(posTag,edge.endPos);
	connEl.appendChild(posTag);
	
	newChild.appendChild(connEl);

	/* pripojeni nove hrany do XML stromu */
	QDomElement refChild = diagramData.lastChildElement("edge");

	/* pokud v diagramu jeste neni zadna hrana lastChildElement vrati null - tento pripad je treba osetrit */
	if (!refChild.isNull())
		diagramData.insertAfter(newChild,refChild);
	else
		diagramData.appendChild(newChild);

	/* pridani nove  hrany do DiagramWidgetu */
	diagramScene->addEdge(edge);
}

void DataWidget::delDataNode(int id)
{
	QDomElement removedNode = findSubelementAttr(diagramData,"node","id",QString().setNum(id));
	QDomNode parent;

	if ( !removedNode.isNull() )
	{
		/* odebrani uzlu z XML struktury */
		parent = removedNode.parentNode();
		parent.removeChild(removedNode);
		
		/* odebrani uzlu z diagramu */
		diagramScene->removeNode(id);
	}
	else
		qWarning() << "DataWidget::delDataNode : node id:" << id << "not found.";
}

EdgeDefinition DataWidget::delDataEdge(int id)
{
	QDomElement removedEdge = findSubelementAttr(diagramData,"edge","id",QString().setNum(id));
	QDomNode parent;
	
	EdgeDefinition result(INVALID_ID,INVALID_ID);

	if ( !removedEdge.isNull() )
	{
		/* ziskani elementu, ktere obsahuji jako atribut id asociovanych hran */
		QDomElement start = removedEdge.firstChildElement("start");
		QDomElement end = removedEdge.firstChildElement("end");
		
		/* ulozeni informace o rozpojenych uzlech */
		result.first = getIntAttribute(start,"nid");
		result.second = getIntAttribute(end,"nid");

		/* odebrani hrany z XML reprezentace */
		parent = removedEdge.parentNode();
		parent.removeChild(removedEdge);

		/* odebrani hrany z diagramu */
		diagramScene->removeEdge(id);
	}
	else
		qWarning() << "$DataWidget::delDataEdge : edge id:" << id << "not found.";

	return result;
}

void DataWidget::changeNodePos(int id, const QPointF & vector)
{
	qDebug() << "$DataWidget::changeNodePos";
	/* najdi uzel */
	QDomElement node = findSubelementAttr(diagramData,"node","id",QString().setNum(id));

	QDomElement position = node.firstChildElement("pos");

	Q_ASSERT(!position.isNull());

	QPointF newPos = getPosValue(position) + vector;

	addVector(position,vector);
}

void DataWidget::changeEdgePos(int id, bool isStart,const QPointF & vector)
{
	/* najdi hranu */
	QDomElement edge = findSubelementAttr(diagramData,"edge","id",QString().setNum(id));

	/* najdi prislusny konec */
	QDomElement edgepoint;
	if (isStart)
	{
		edgepoint = edge.firstChildElement("start");
	}
	else
	{
		edgepoint = edge.firstChildElement("end");
	}
	
	Q_ASSERT(!edgepoint.isNull());
	
	/* najdi pozicni tag */
	QDomElement position = edgepoint.firstChildElement("pos");

	Q_ASSERT(!position.isNull());
	
	QPointF newPos = getPosValue(position) + vector;

	addVector(position,vector);
}

void DataWidget::changeEdgeAssociation(int id, bool isStart, int toNode)
{
	/* najdi hranu */
	QDomElement edge = findSubelementAttr(diagramData,"edge","id",QString().setNum(id));
	
	/* zmen asociovany uzel */
	if (isStart)
		setIntAttribute(edge.firstChildElement("start"),"nid",toNode);
	else
		setIntAttribute(edge.firstChildElement("end"),"nid",toNode);
}

QDomElement DataWidget::disconnectEdgeFromNode(int edgeID, bool start, int nodeID)
{
	/* pocet konexi uzlu */
	QDomElement targetNode = findSubelementAttr(diagramData,"node","id",QString().setNum(nodeID));
	QDomElement connections = targetNode.firstChildElement("connections");

	Q_ASSERT(!connections.isNull());
	
	int connCount = connections.childNodes().count();

	/* v pripade uspesneho smazani vrati naslednika smazaneho tagu
	 * pokud naslednik neexistuje, nebo neni nalezeno spojeni ktere se ma smazat vraci Null */
	QDomElement result = QDomElement();

	if ( connCount > 1 )
	{
		qDebug() << "$disconnectEdgeFromNode: connCount > 1";
		QDomElement removed;

		if (start)
		{
			qDebug() << "$disconnectEdgeFromNode: removing <starts>";
			removed = findSubelementVal(connections,"starts",QString().setNum(edgeID));

			/* kvuli precislovani parametru */
			result = removed.nextSiblingElement("starts");
		}
		else
		{
			qDebug() << "$disconnectEdgeFromNode: removing <ends>";
			removed = findSubelementVal(connections,"ends",QString().setNum(edgeID));
		}

		connections.removeChild(removed);
	}
	else
		targetNode.removeChild(connections);

	return result;
}

QDomElement DataWidget::connectEdgeToNode(int edgeID, bool start, int nodeID)
{
	/* najdi uzel */
	QDomElement node = findSubelementAttr(diagramData,"node","id",QString().setNum(nodeID));

	QDomElement connections = node.firstChildElement("connections");
	if (connections.isNull())
	{
		connections = XMLdata.createElement("connections");
		node.appendChild(connections);
	}

	/* prevod cisla hrany na retezec */
	QString edgeNum;
	edgeNum.setNum(edgeID);

	/* pridani podelementu */
	if (start)
		return addChildElement(connections,"starts",edgeNum);
	else
		return addChildElement(connections,"ends",edgeNum);
}

void DataWidget::addVector(QDomElement & pos, QPointF vector)
{
	QPointF oldPos = getPosValue(pos);

	setPosition(pos,oldPos+vector);
}

void DataWidget::setPosition(QDomElement & pos, QPointF point)
{
	/* prevod realneho cisla na retezec */
	QString strX;
	QString strY;

	strX.setNum(point.x(),'g');
	strY.setNum(point.y(),'g');

	/* nalezeni podelementu */
	QDomElement subX = pos.firstChildElement("x");
	QDomElement subY = pos.firstChildElement("y");

	/* -> pokud podelement neexistuje, prida se novy s prislusnym obsahem 
	 * -> obsah existujiciho podelementu nahrazen */

	if (subX.isNull())
		addChildElement(pos,"x",strX);
	else
		subX.replaceChild(XMLdata.createTextNode(strX),subX.firstChild());
		
	if (subY.isNull())
		addChildElement(pos,"y",strY);
	else
		subY.replaceChild(XMLdata.createTextNode(strY),subY.firstChild());
}

QDomElement DataWidget::addChildElement(QDomElement & parent, QString childName, QString content)
{
	QDomElement newTag = XMLdata.createElement(childName);
	newTag.appendChild(XMLdata.createTextNode(content));
	
	Q_ASSERT(!parent.isNull());
	
	parent.appendChild(newTag);
	return newTag;
}

void DataWidget::resetPredicate(QDomElement & elem)
{
	QDomElement setTag = elem.firstChildElement("set");

	while( !setTag.isNull() )
	{
		elem.removeChild(setTag);
		setTag = setTag.nextSiblingElement("set");
	}
}

const NodeStructure DataWidget::loadNode(const QDomElement & elem) const
{
	NodeStructure result;

	char nodeType = detNodeType(elem.firstChildElement("type"));

	result.setData(NodeStructure::nodeID,getIntAttribute(elem,"id")); 
	result.setData(NodeStructure::nodeLabel,getStrValue(elem.firstChildElement("label"))); 
	result.setData(NodeStructure::nodePosition,getPosValue(elem.firstChildElement("pos")));
	result.setData(NodeStructure::nodeType,nodeType);

	QDomElement taskState = elem.firstChildElement("state");
	if ( !taskState.isNull() )
		result.setData(NodeStructure::nodeState,getStrValue(taskState));
	
	if (nodeType & NST_PREDICATE)
	{
		QDomElement setElement = elem.firstChildElement("set");
		QStringList predSet;
		while( !setElement.isNull() )
		{
			predSet.append(getStrValue(setElement));
			setElement = setElement.nextSiblingElement("set");
		}
		
		result.setData(NodeStructure::nodePredicateSet,predSet);
	}

	return result;
}

const EdgeStructure DataWidget::loadEdge(const QDomElement & elem) const
{
	EdgeStructure result;
	
	result.id = getIntAttribute(elem,"id");
	result.purpose = detEdgePurpose(elem.firstChildElement("purpose")); 

	QDomElement startPoint = elem.firstChildElement("start");
	QDomElement endPoint = elem.firstChildElement("end");

	Q_ASSERT(!startPoint.isNull() && !endPoint.isNull());
	
	result.startNodeID = getIntAttribute(startPoint,"nid");
	result.endNodeID = getIntAttribute(endPoint,"nid");

	result.startPos = getPosValue(startPoint.firstChildElement("pos"));
	result.endPos = getPosValue(endPoint.firstChildElement("pos"));

	return result;
}
