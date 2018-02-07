#include "Edge.h"
#include "Node.h"
#include "CustomEvents.h"
#include "Edgepoint.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
const qreal Pi = 3.14159;

Edge::Edge()
{
	edgeId = INVALID_ID;
}

Edge::Edge(const QLineF & line, int id, QGraphicsScene * scene, QObject * host)
:QGraphicsLineItem(line,0),edgeId(id)
{
	Q_ASSERT(scene != 0);
	Q_ASSERT(host != 0);
	eventTarget = host;
	
	start = new Edgepoint(line.p1(),EDGE_START,this);
	end = new Edgepoint(line.p2(),EDGE_END,this);

	/* hrana se musi pridat na scenu, aby mohly byt nainstalovany event filtry */
	scene->addItem(this);

	start->installSceneEventFilter(this);
	end->installSceneEventFilter(this);

	setZValue(EDGE_Z_VALUE);
	setAcceptedMouseButtons(Qt::LeftButton);
}

bool Edge::isValid()
{
	return !(edgeId == INVALID_ID);
}

void Edge::setPos(const QPointF & newPos, bool isStart)
{
	hide();
	if (isStart)
	{
		start->setPos(newPos);
		setLine(QLineF(newPos,line().p2()));
		end->updateAngle(getAngle());	/* pouze kvuli prekresleni sipky */ 
	}
	else
	{
		end->setPos(newPos);
		setLine(QLineF(line().p1(),newPos));
		end->updateAngle(getAngle());	/* pouze kvuli prekresleni sipky */
	}
	show();
}

void Edge::moveBy(const QPointF & vector, bool isStart)
{
	hide();
	if(isStart)
	{
		start->moveBy(vector);	
		setLine(QLineF(line().p1() + vector,line().p2()));
		end->updateAngle(getAngle());	/* pouze kvuli prekresleni sipky */
	}
	else
	{
		end->moveBy(vector);
		setLine(QLineF(line().p1(),line().p2() + vector));
		end->updateAngle(getAngle());	/* pouze kvuli prekresleni sipky */
	}
	show();
}

void Edge::stickTo(const DiagramElement * another, bool isStart)
{
	QPointF oldPos;
	QPointF vector;

	hide();
	if(isStart)
	{
		oldPos = start->scenePos();
		start->stick(another);

		vector = start->scenePos() - oldPos;
		
		setLine(QLineF(line().p1() + vector,line().p2()));
		end->updateAngle(getAngle());	/* pouze kvuli prekresleni sipky */
	}
	else
	{
		oldPos = end->scenePos();
		end->stick(another);

		vector = end->scenePos() - oldPos;
		
		setLine(QLineF(line().p1(),line().p2() + vector));
		end->updateAngle(getAngle());	/* pouze kvuli prekresleni sipky */
	}
	show();
}

void Edge::setEdgeOrientation(EdgeOrientation orientation)
{
	if (!start || !end)
	{
		qWarning() << "$Edge::setEdgeOrientation : edgepoints not initialized";
		return;
	}

	switch(orientation)
	{
		case NoOrientation:
			start->setShape(Edgepoint::Point);
			end->setShape(Edgepoint::Point);
		break;
		case Forward:
			start->setShape(Edgepoint::Point);
			end->setShape(Edgepoint::ArrowPos);
			end->updateAngle(getAngle());
		break;
		case Backward:
			start->setShape(Edgepoint::ArrowPos);
			start->updateAngle(getAngle());
			end->setShape(Edgepoint::Point);
		break;
		default:
		Q_ASSERT(false);
	}
}

void Edge::setPointColor(QColor color)
{
	Q_ASSERT(start && end);

	start->setColor(color);
	end->setColor(color);
}

int Edge::getID()
{
	return edgeId;
}

int Edge::connectedNodeID(bool isStart, QList<Node*> nodeList)
{
	Edgepoint * tested;

	if (isStart)
		tested = start; 
	else
		tested = end;
	
	int result = 0;

	foreach(Node * actNode, nodeList)
	{
		if ( tested->collidesWithItem(actNode) )
		{
			if (result == 0)
			{
				/* prvni zaznamenana kolize */
				result = actNode->getID();
			}
			else
			{
				/* kolize s vice nez jednim uzlem */
				return INVALID_ID;
			}
		}
	}
	
	return result;
}

QPainterPath Edge::shape() const
{
	QPainterPath result;

	result.addPath(QGraphicsLineItem::shape());
	result = result.united(start->mapToScene(start->shape()));
	result = result.united(end->mapToScene(end->shape()));

	return result;
}

QRectF Edge::boundingRect() const
{
	return childrenBoundingRect();
}

double Edge::getAngle()
{
	double angle = acos(line().dx()/line().length());
	if (line().dy() > 0)
		angle = (2*Pi) - angle;
	return angle;
}

bool Edge::sceneEventFilter(QGraphicsItem * watched, QEvent * event)
{
	Q_ASSERT(watched);

	if ( watched == start )
	{
		if ( event->type() == QEvent::GraphicsSceneMousePress )
		{
			startGrabPos = start->pos();
			return true;
		}

		if ( event->type() == QEvent::GraphicsSceneMouseRelease )
		{
			qDebug() << "$Edge::sceneEventFilter : start mouse release";
			handleClickEvent(EDGE_START,start->pos());		
			return true;
		}
		
		if ( event->type() == QEvent::GraphicsSceneMouseMove )
		{
			QGraphicsSceneMouseEvent * myEvent = static_cast<QGraphicsSceneMouseEvent *>(event);

			/* udrzeni hrany uvnitr sceny */
			QPointF newPos = myEvent->scenePos();
			QRectF rect = scene()->sceneRect();
			if (!rect.contains(newPos))
			{
				newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
				newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
			}

			setLine(QLineF(newPos,line().p2()));
			end->updateAngle(getAngle());
			start->setPos(newPos);
			return true;
		}
	}

	if ( watched == end )
	{
		if ( event->type() == QEvent::GraphicsSceneMousePress )
		{
			endGrabPos = end->pos();
			return true;
		}

		if ( event->type() == QEvent::GraphicsSceneMouseRelease )
		{
			qDebug() << "$Edge::sceneEventFilter : end mouse release";
			handleClickEvent(EDGE_END,end->pos());		
			return true;
		}

		if ( event->type() == QEvent::GraphicsSceneMouseMove )
		{
			QGraphicsSceneMouseEvent * myEvent = static_cast<QGraphicsSceneMouseEvent *>(event);

			/* udrzeni hrany uvnitr sceny */
			QPointF newPos = myEvent->scenePos();
			QRectF rect = scene()->sceneRect();
			if (!rect.contains(newPos))
			{
				newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
				newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
			}
			
			setLine(QLineF(line().p1(),newPos));
			end->updateAngle(getAngle());
			end->setPos(newPos);
			return true;
		}
	}

	return true;
}

void Edge::handleClickEvent(bool isStart, QPointF pos)
{

	/* odesila se informace o tom, ze:
	 * bud 1) uzivatel pohnul hranou edgeId
	 * 	zacatkam/koncem (dle isStart) o vector 
	 * nebo 2) uzivatel kliknul na hranu levym tlacitkem */

	QPointF vector;

	if(isStart)
	{
		vector = pos - startGrabPos;
		qDebug() << "$Edge::handleClickEvent : (start) vector = " << vector;
		if ( !vector.isNull() )
		{
			QVariantList package;
			package.append(isStart);
			package.append(vector);

			DiagramEvent myEvent(EdgeMoved,edgeId,QVariant(package));
			QApplication::sendEvent(eventTarget,&myEvent);
		}
	}
	else
	{
		vector = pos - endGrabPos;
		qDebug() << "$Edge::handleClickEvent : (end) vector = " << vector;
		if ( !vector.isNull() )
		{
			QVariantList package;
			package.append(isStart);
			package.append(vector);

			DiagramEvent myEvent(EdgeMoved,edgeId,QVariant(package));
			QApplication::sendEvent(eventTarget,&myEvent);
		}
	}
}
