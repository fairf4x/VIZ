#include "EditWidget.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"
#include "Node.h"
#include "CustomEvents.h"

#include <QFile>
#include <QtDebug>

/* defaultni rozmer diagramu */
#define DIAGRAM_SIZE 1000

EditWidget::EditWidget(QWidget * parent)
: QGraphicsView(parent)
{
	diagram = new DiagramWidget(DIAGRAM_SIZE,DIAGRAM_SIZE,this);
	xmlData = new DataWidget(diagram);
	
	this->setScene(diagram);

	/* inicializace promennych */
	changed = false;
	nameChecker.setPattern(QString("^([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[-_]){,%1}$").arg(MAX_LABEL_LENGTH));
	editMode = AddRect;
}

void EditWidget::reset()
{
	xmlData->loadData(QDomDocument());
	changed = false;
}

bool EditWidget::wasChanged()
{
	return changed;
}

void EditWidget::setEditMode(EditMode mode)
{
	editMode = mode;
	
	switch(mode)
	{
		case AddRect:
		case AddEllipse:
		case Delete:
			diagram->setDiagramMode(DiagramWidget::DetectClick);
		break;
		case AddEdge:
			diagram->setDiagramMode(DiagramWidget::InsertLine);
		break;
		default:
			Q_ASSERT(false);
	}
}

void EditWidget::setChanged(bool value)
{
	changed = value;
}

void EditWidget::handleExternChange(int changeCode)
{
	setChanged(true);
	emit sceneChanged(changeCode);
}

const DataWidget * EditWidget::xmlDataPointer()
{
	Q_ASSERT(xmlData);
	return xmlData;	
}

void EditWidget::handleNodeMove(int id, QPointF vector)
{
	/* kdyz je uzel "upusten" nekam na plochu, zapise se prislusna zmena souradnic i do XML 
	 * */

	xmlData->changeNodePos(id,vector);

	QList<DataEdgepoint> assocEdges = xmlData->associatedEdgepoints(id);

	int actID;
	bool actBool;

	/* zapsani zmenenych souradnic vsech pripojenych hran */
	for(int i=0; i < assocEdges.size(); ++i)
	{
		/* zpracovani jednoho edgepointu */

		actID = assocEdges.at(i).first;
		actBool = assocEdges.at(i).second;
		
		xmlData->changeEdgePos(actID,actBool,vector);
	}

	changed = true;
}

void EditWidget::handleEdgeMove(int id, bool isStart, QPointF vector)
{
	/* pohyb jednoho edgepointu */
	/* zjisti kolidujici uzel z diagramu */
	Q_ASSERT(diagram);
	int diagNode = diagram->collidingNodeID(id,isStart);

	/* nacti hranu ktere se prepojeni tyka */
	Q_ASSERT(xmlData);
	EdgeStructure reconnectedEdge = xmlData->getEdgeDescription(id);

	int argNum = INVALID_ARGN;

	/* overeni zda je prepojeni legalni */
	switch( verifyReconnection(reconnectedEdge,diagNode,isStart,argNum) )
	{
		case NoChange:
			/* posun hranu zpatky */
			diagram->translateEdge(id,isStart,-vector);
		break;
		case PosChange:
			/* zmen pouze pozici */
			xmlData->changeEdgePos(id,isStart,vector);
			changed = true;
		break;
		case PosAssocChange:
			/* prepoj hranu - prepisou se vsechny souvisejici zaznamy v XML */
			changeConnection(reconnectedEdge,isStart,diagNode,argNum);
			changed = true;
		break;
		default:
			/* nerozpoznany vysledek prepojeni */
			Q_ASSERT(false);
	}
}

void EditWidget::handleNodeDrag(int id, QPointF vector)
{
	/* pouze predava DiagramWidgetu instrukce aby si prekreslil situaci 
	 * (stara se o prekresleni pripojenych hran)
	 * */
	Q_ASSERT(xmlData);
	Q_ASSERT(diagram);
	QList<DataEdgepoint> assocEdges = xmlData->associatedEdgepoints(id);

	int actID;
	bool actBool;

	/* posun vsechny edgepointy */
	foreach(DataEdgepoint point,assocEdges)
	{
		actID = point.first;
		actBool = point.second;
		diagram->translateEdge(actID,actBool,vector);
	}
}

void EditWidget::handleNodeReshape(int id)
{
	/* zjisti seznam pripojenych hran */
	Q_ASSERT(xmlData);
	Q_ASSERT(diagram);
	QList<DataEdgepoint> edgepoints = xmlData->associatedEdgepoints(id);

	int actID;
	bool actBool;

	qDebug() << "handleNodeReshape for:" << id;
	foreach(DataEdgepoint point, edgepoints)
	{
		actID = point.first;
		actBool = point.second;
		qDebug() << "edge:" << actID;
		diagram->stickEdgeToNode(actID,actBool,id);
	}
}

void EditWidget::loadXMLdata(QDomDocument diagramDocument)
{
	xmlData->loadData(diagramDocument);
	changed = false;
}

bool EditWidget::event(QEvent * event)
{
	if (event->type() == DiagramLeftClick)
	{
		qDebug() << "$DiagramLeftClick";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
			
		qDebug() << "$newNodeID:" << myEvent->itemID();
		
		Q_ASSERT(myEvent->data().canConvert(QVariant::PointF));

		QPointF newNodePos = myEvent->data().toPointF();

		if (editMode == AddRect)
			defineRectangleNode(newNodePos,myEvent->itemID());

		if (editMode == AddEllipse)
			defineEllipseNode(newNodePos,myEvent->itemID());

		changed = true;

		return true;
	}

	if (event->type() == DiagramRightClick)
	{
		/* nepouzito */
		return true;
	}

	if (event->type() == NodeLeftClick)
	{
		qDebug() << "$NodeLeftClick";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
		qDebug() << "$clicked node ID:" << myEvent->itemID();

		if (editMode == Delete)
			deleteNode(myEvent->itemID());

		return true;
	}

	if (event->type() == NodeRightClick)
	{
		qDebug() << "$NodeRightClick";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
		qDebug() << "$clicked node ID:" << myEvent->itemID();

		displayInfo(myEvent->itemID()); 

		return true;
	}

	if (event->type() == EdgeLeftClick)
	{
		qDebug() << "$EdgeLeftClick";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
		qDebug() << "$clicked edge ID:" << myEvent->itemID();

		if (editMode == Delete)
			deleteEdge(myEvent->itemID());

		return true;
	}

	if (event->type() == EdgeRightClick)
	{
		/* nepouzito */
		return true;
	}

	if (event->type() == NodeMoved)
	{
		qDebug() << "$NodeMoved";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
		qDebug() << "$node ID:" << myEvent->itemID();

		Q_ASSERT(myEvent->data().canConvert(QVariant::PointF));
		QPointF vector = myEvent->data().toPointF();
		qDebug() << "$vector:" << vector;

		handleNodeMove(myEvent->itemID(),vector);

		return true;
	}

	if (event->type() == EdgeMoved)
	{
		qDebug() << "$EdgeMoved";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
		qDebug() << "$edge ID:" << myEvent->itemID();

		Q_ASSERT(myEvent->data().canConvert(QVariant::List));
		QVariantList package = myEvent->data().toList();
		bool isStart = package.takeFirst().toBool();
		QPointF vector = package.takeFirst().toPointF();
		qDebug() << "$vector:" << vector;

		handleEdgeMove(myEvent->itemID(),isStart,vector);	
		
		return true;
	}

	if (event->type() == NodeDrag)
	{
		qDebug() << "$NodeDrag";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
		qDebug() << "$node ID:" << myEvent->itemID();
		
		Q_ASSERT(myEvent->data().canConvert(QVariant::PointF));
		QPointF vector = myEvent->data().toPointF();
		qDebug() << "$vector:" << vector;

		handleNodeDrag(myEvent->itemID(),vector);

		return true;
	}

	if (event->type() == EdgeDefined)
	{
		qDebug() << "$EdgeDefined";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
		qDebug() << "$new edgeID:" << myEvent->itemID();

		Q_ASSERT(myEvent->data().canConvert(QVariant::List));

		QVariantList package = myEvent->data().toList();
		int startID =	package.takeFirst().toInt();
		int endID =	package.takeFirst().toInt();
		QPointF startPos = package.takeFirst().toPointF();
		QPointF endPos = package.takeFirst().toPointF();
		qDebug() << "$start: " << startID << "," << startPos;
		qDebug() << "$end: " << endID << "," << endPos;

		if (editMode == AddEdge)
			defineEdge(startID,endID,startPos,endPos,myEvent->itemID());
		else
			qWarning() << "$EditWidget::event : EdgeDefined event in wrong mode";

		return true;
	}
	
	if (event->type() == NodeReshaped)
	{
		qDebug() << "$NodeReshaped";
		DiagramEvent * myEvent = static_cast<DiagramEvent *>(event);
		qDebug() << "$node ID:" << myEvent->itemID();

/*		handleNodeReshape(myEvent->itemID());*/

		return true;
	}

	return QGraphicsView::event(event);
}

char EditWidget::determineEdgePurpose(char startType, char endType)
{
	/*	C - class		
		V - variable		"X" edge is forbiden
		O - object		"A" asociation edge
		P - predicate		"I" inheritance edge	

	  C V O P
	C I X X X
	V X X X X
	O X X X X
	P A A A X	*/

	if ( startType == endType )
	{
		if ( startType == NST_CLASS )
			return DEP_INHERITANCE;
		else
			return DEP_NO_PURPOSE;
	}
	else
	{
		if ( (startType == NST_PREDICATE) && (endType != NST_PREDICATE) )
			return DEP_ASSOCIATION;
		else
			return DEP_NO_PURPOSE;
	}
}

void EditWidget::defineEdge(int startID, int endID, QPointF startPoint, QPointF endPoint, int newID)
{
	EdgeStructure newEdge;
	newEdge.id = newID;
	newEdge.startPos = startPoint;
	newEdge.endPos = endPoint;
	newEdge.startNodeID = startID;
	newEdge.endNodeID = endID;

	int argNum = INVALID_ARGN;
	/* pokud hrana projde testem tak vytvor spojeni */
	if ( verifyEdge(newEdge,argNum) )
	{
		makeConnection(newEdge,argNum);
		changed = true;
	}
}

void EditWidget::deleteEdge(int edgeID)
{
	char edgePurpose = xmlData->readEdgePurpose(edgeID);
	/* odebira se hrana */
	deleteConnection(edgeID);
	changed = true;

	/* hlaseni o tom co se stalo */
	switch(edgePurpose)
	{
		case DEP_ASSOCIATION:
			emit sceneChanged(EditWidget::AssocEdgeDeleted);
		break;
		case DEP_INHERITANCE:
			emit sceneChanged(EditWidget::InherEdgeDeleted);
		break;
		default:
			Q_ASSERT(false);
	}
	return;
}

void EditWidget::deleteNode(int nodeID)
{
	/* odebira se uzel */
	char nodeType = xmlData->nodeType(nodeID);

	/* odebrani vsech spojeni */
	QList<int> edges = xmlData->associatedEdges(nodeID);
	foreach(int edgeID, edges)
		deleteConnection(edgeID);
	
	xmlData->delDataNode(nodeID);

	changed = true;

	/* hlaseni o tom co se stalo */
	switch(nodeType)
	{
		case NST_PREDICATE:
			emit sceneChanged(EditWidget::EllipseNodeDeleted);
		break;
		case NST_VARIABLE:
		case NST_OBJECT:
		case NST_CLASS:
			emit sceneChanged(EditWidget::RectNodeDeleted);
		break;
		default:
			Q_ASSERT(false);
	}
}

bool EditWidget::verifyEdge(EdgeStructure & edge, int & argNum)
{
	Q_UNUSED(argNum);

	char startType = xmlData->nodeType(edge.startNodeID);
	char endType = xmlData->nodeType(edge.endNodeID);

	/* odvozeni typu hrany podle typu pripojenych uzlu */
	if ( (startType != NST_PREDICATE) && (endType == NST_PREDICATE) )
	{
		/* zajisteni invariantu ze predikat ma pouze vystupni hrany */
		edge.switchOrientation();
		edge.purpose = determineEdgePurpose(endType,startType);		
	}
	else
	{
		edge.purpose = determineEdgePurpose(startType,endType);
	}

	/* zakazani hran ktere nejsou spravneho typu */
	if ( !(edge.purpose & allowedEdgeMask) )
	{
		qWarning() << "This edge is not allowed here.";
		return false;
	}

	/* vsechny testy prosly */
	return true;
}
