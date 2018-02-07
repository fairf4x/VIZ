#include "DiagramWidget.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"
#include "Edgepoint.h" /* jen kvuli testu souradnic */
#include "Node.h"
#include "Edge.h"
#include <QtDebug>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include "CustomEvents.h"

DiagramWidget::DiagramWidget(int width, int height, QWidget * parent)
:QGraphicsScene(parent)
{
	setSceneRect(0,0,width,height);
	
	/* inicializace privatnich promennych*/
	tempLine = NULL;
	myMode = DetectClick; 
}

void DiagramWidget::reset()
{
	dgwNodes.clear();
	dgwEdges.clear();
	this->clear();
	
	tempLine = 0;
}

void DiagramWidget::setDiagramMode(DiagramMode mode)
{
	switch(mode)
	{
		case InsertLine:
		case DetectClick:
		case MoveItems:
			myMode = mode;
		break;
		default:
			Q_ASSERT(false);
	}
}

void DiagramWidget::setNodeVisible(int nodeID, bool visible)
{
	Node * target = dgwNodes.value(nodeID,NULL);

	if (target == NULL)
	{
		qDebug() << "$DiagramWidget::setNodeVisible : node not found";
		return;
	}

	target->setEnabled(visible);
	target->setVisible(visible);
}

void DiagramWidget::setEdgeVisible(int edgeID, bool visible)
{
	Edge * target = dgwEdges.value(edgeID,NULL);

	if (target == NULL)
	{
		qDebug() << "DiaramWidget::setEdgeVisible : edge not found";
		return;
	}

	target->setEnabled(visible);
	target->setVisible(visible);
}

void DiagramWidget::addNode(const NodeStructure & node)
{
	qDebug() << "$DiagramWidget::addNode";
	Node * NewNode = new Node(node.readData(NodeStructure::nodeID).toInt(), node.shape(),this,this->parent());

    char nodeType = node.readData(NodeStructure::nodeType).toChar().toLatin1();
	int nodeID = node.readData(NodeStructure::nodeID).toInt();

	dgwNodes.insert(nodeID,NewNode);
	NewNode->setPos(node.readData(NodeStructure::nodePosition).toPointF());
	NewNode->setLabel(node.readData(NodeStructure::nodeLabel).toString());
	NewNode->setColor(node.color());

	if ( nodeType == NST_PREDICATE )
	{
		QStringList predSet = node.readData(NodeStructure::nodePredicateSet).toStringList();

		for (int i = 0; i<predSet.size(); ++i)
			NewNode->setLayer(layerColor(predSet.at(i)));
	}
}

void DiagramWidget::addEdge(const EdgeStructure & edge)
{
	qDebug() << "$DiagramWidget::addEdge";	
	qDebug() << "$pos:" << edge.startPos << edge.endPos;
	Edge * NewEdge = new Edge(QLineF(edge.startPos,edge.endPos),edge.id,this,this->parent());

	dgwEdges.insert(edge.id,NewEdge);
	NewEdge->setEdgeOrientation(edge.orientation());
	NewEdge->setPen(edge.pen());
	NewEdge->setPointColor(edge.pen().color());

}

void DiagramWidget::removeNode(int id)
{
	Q_ASSERT(dgwNodes.find(id) != dgwNodes.end());

	Node * actNode = dgwNodes.take(id);

	removeItem(actNode);
	delete actNode;
}

void DiagramWidget::removeEdge(int id)
{
	Q_ASSERT(dgwEdges.find(id) != dgwEdges.end());

	Edge * actEdge = dgwEdges.take(id);

	removeItem(actEdge);
	delete actEdge;
}

void DiagramWidget::updateNode(const NodeStructure & update)
{
	int nodeID = update.readData(NodeStructure::nodeID).toInt();

	Node * updatedNode = dgwNodes.value(nodeID,NULL);
	
	if ( updatedNode == NULL )
	{
		qDebug() << "$DiagramWidget::updateNode : node not found in diagram";
		return;
	}

	updatedNode->hide();

	QVariant newData;
	
	newData = update.readData(NodeStructure::nodeLabel);
	if ( !newData.isNull() )
		updatedNode->setLabel(newData.toString());		

	newData = update.readData(NodeStructure::nodePredicateSet);
	if ( !newData.isNull() )
	{
		updatedNode->clearLayers();
		
		QStringList newLayers = newData.toStringList();
		for (int i=0; i < newLayers.size(); ++i)
			updatedNode->setLayer(layerColor(newLayers.at(i)));
	}

	newData = update.readData(NodeStructure::nodeState);
	if (!newData.isNull() )
		updatedNode->setColor(stateColor(newData.toString()));

	updatedNode->show();
}

void DiagramWidget::translateEdge(int id, bool start, QPointF vector)
{
	Edge * translatedEdge = dgwEdges.value(id,NULL);

	if (translatedEdge != NULL)
		translatedEdge->moveBy(vector,start);
	else
		qDebug() << "$DiagramWidget::translateEdge : edge not found in diagram";
}

void DiagramWidget::stickEdgeToNode(int edgeID, bool start, int nodeID)
{
	Edge * connectedEdge = dgwEdges.value(edgeID,NULL);
	Node * targetNode = dgwNodes.value(nodeID,NULL);

	/* funkce stickTo zajistuje pritazeni hrany edgeID k uzlu s nodeID 
	 * parametr start urcuje, ktery konec hrany bude "pritahnut" */
	if ( (connectedEdge != NULL) && (targetNode != NULL) )
		connectedEdge->stickTo(targetNode, start);
	else
		qDebug() << "$DiagramWidget::stickEdgeToNode : edge or node not found";
}

int DiagramWidget::collidingNodeID(int edgeID, bool start)
{
	Edge * targetEdge = dgwEdges.value(edgeID,NULL);

	if (targetEdge != NULL)
		return targetEdge->connectedNodeID(start,dgwNodes.values());
	else
		return INVALID_ID;
}

int DiagramWidget::collidingNodeID(QPointF coord)
{
	/* funkce nodeAt vraci NULL v pripade, ze bod urceny coord neni obsazen v zadnem uzlu */
	if (Node * node = nodeAt(coord))
		return node->getID();
	else
		return INVALID_ID; 
}

int DiagramWidget::collidingEdgeID(QPointF coord)
{
	/* funkce edgeAt vraci NULL v pripade, ze bod urceny coord nekoliduje s zadnou hranou */
	if (Edge * edge = edgeAt(coord))
		return edge->getID();
	else
		return INVALID_ID; 
}

Node * DiagramWidget::nodeAt(const QPointF & pos)
{
	NodeContainer::iterator iter;
	Node * actNode;
	QPointF localPos;	/* lokalni pozice vzhledem k pozici uzlu */

	for (iter = dgwNodes.begin(); iter != dgwNodes.end(); ++iter)
	{
		actNode = iter.value();	
		localPos = pos - actNode->pos();
		if ( actNode->contains(localPos) )
			return actNode;
	}
	return NULL;
}

Edge * DiagramWidget::edgeAt(const QPointF & pos)
{
	EdgeContainer::iterator iter;
	Edge * actEdge;
	
	for (iter = dgwEdges.begin(); iter != dgwEdges.end(); ++iter)
	{
		actEdge = iter.value();
		if (actEdge->contains(pos))
			return actEdge;
	}
	return NULL;
}

QString DiagramWidget::layerColor(QString predicateSet)
{
	if (predicateSet == NSPS_PRECOND)
		return LC_PRECOND;
	
	if (predicateSet == NSPS_EFFECT_POS)
		return LC_EFFECT_POS;

	if (predicateSet == NSPS_EFFECT_NEG)
		return LC_EFFECT_NEG;

	return  LC_DEFAULT;
}

QString DiagramWidget::stateColor(QString predicateState)
{
	if ( predicateState == NSTS_INIT )
		return NC_INIT_PREDICATE;

	if ( predicateState == NSTS_GOAL )
		return NC_GOAL_PREDICATE;

	return NC_DEFAULT;
}

int DiagramWidget::newNodeID()
{
	QSet<int> idSet = QSet<int>::fromList(dgwNodes.keys());

	int newId = qrand();
	while(idSet.contains(newId))
		newId = qrand();

	return newId;
}

int DiagramWidget::newEdgeID()
{
	QSet<int> idSet = QSet<int>::fromList(dgwEdges.keys());

	int newId = qrand();
	while(idSet.contains(newId))
		newId = qrand();

	return newId;
}

void DiagramWidget::leftClick(QPointF pos)
{
    if ( itemAt(pos,QTransform()) == 0)
	{
		DiagramEvent myEvent(DiagramLeftClick,newNodeID(),pos);
		QApplication::sendEvent(this->parent(),&myEvent);
		return;
	}

	int edgeID = collidingEdgeID(pos);
	if ( edgeID != INVALID_ID )
	{
		DiagramEvent myEvent(EdgeLeftClick,edgeID);
		QApplication::sendEvent(this->parent(),&myEvent);
		return;
	}

	int nodeID = collidingNodeID(pos);
	if ( nodeID != INVALID_ID )
	{
		DiagramEvent myEvent(NodeLeftClick,nodeID);
		QApplication::sendEvent(this->parent(),&myEvent);
	}
}

void DiagramWidget::rightClick(QPointF pos)
{
    if ( itemAt(pos,QTransform()) == 0)
	{
		/* event neni programem vyuzivan */
		DiagramEvent myEvent(DiagramRightClick,INVALID_ID,pos);
		QApplication::sendEvent(this->parent(),&myEvent);
		return;
	}

	int edgeID = collidingEdgeID(pos);
	if ( edgeID != INVALID_ID )
	{
		/* event neni programem vyuzivan */
		DiagramEvent myEvent(EdgeRightClick,edgeID);
		QApplication::sendEvent(this->parent(),&myEvent);
		return;
	}

	int nodeID = collidingNodeID(pos);
	if ( nodeID != INVALID_ID )
	{
		DiagramEvent myEvent(NodeRightClick,nodeID);
		QApplication::sendEvent(this->parent(),&myEvent);
	}
}

void DiagramWidget::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	if ( (event->button() != Qt::LeftButton) && (event->button() != Qt::RightButton) )
		return;

	switch(myMode)
	{
		case InsertLine:
			if ( event->button() == Qt::LeftButton )
			{
				/* vytvor docasnou caru */
				tempLine = new QGraphicsLineItem(QLineF(event->scenePos(),
									 event->scenePos()));
				tempLine->setPen(QPen(QColor(TEMP_LINE_COLOR),TEMP_LINE_WIDTH));
				tempLine->setZValue(TEMP_LINE_Z_VALUE);
				addItem(tempLine);
			}
			else
				rightClick(event->scenePos());
		break;
		case DetectClick:
			if ( event->button() == Qt::LeftButton )
				leftClick(event->scenePos());
			else
				rightClick(event->scenePos()); 
		break;
		case MoveItems:
			/* event bude zpracovan defaultnim handlerem pokud jde o leve tlacitko */
			if (event->button() != Qt::LeftButton)
				return;
		break;
		default:
			Q_ASSERT(false);	
	}
	
	qDebug() << "$DiagramWidget::mousePressEvent : calling QGraphicsScene::mousePressEvent";
	QGraphicsScene::mousePressEvent(event);
}

void DiagramWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	if ( (event->button() != Qt::LeftButton) && (event->button() != Qt::RightButton) )
		return;

	if ( (tempLine != 0) && (myMode == InsertLine) )
	{
		QPointF startPos = tempLine->line().p1();
		QPointF endPos = tempLine->line().p2();
		int startID = collidingNodeID(startPos);
		int endID = collidingNodeID(endPos);

		removeItem(tempLine);
		delete tempLine;

		/* kontrola hrany */
		if (	(startID != INVALID_ID) &&
			(endID != INVALID_ID) &&
			(startID != endID) )
		{		
			QVariantList package;
			package.append(startID);
			package.append(endID);
			package.append(startPos);
			package.append(endPos);
			
			qDebug() << "$DiagramWidget::mouseReleaseEvent : edge defined";
			DiagramEvent myEvent(EdgeDefined,newEdgeID(),QVariant(package));
			QApplication::sendEvent(parent(),&myEvent);
		}
	}

	tempLine = 0;

	QGraphicsScene::mouseReleaseEvent(event);
}

void DiagramWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	switch(myMode)
	{
		case InsertLine:
			/* obstarej pohyb tempLine */
			if ( tempLine != 0 )
			{
				QLineF newLine(tempLine->line().p1(), event->scenePos());
				tempLine->setLine(newLine);
			}
		break;
		case MoveItems:
		case DetectClick:
			/* defaultni zpracovani eventu pouze pri stisknutem levem tlacitku */
			if (event->buttons() == Qt::LeftButton || event->buttons() == Qt::NoButton)
				QGraphicsScene::mouseMoveEvent(event);
		default:
			return;
	}	
	
}
