#include <QtGui>
#include <QGraphicsLineItem>
#include "Edgepoint.h"
#include "CustomEvents.h"
#include <math.h>
const qreal Pi = 3.14159;

Edgepoint::Edgepoint(const QPointF & pos, bool isStart, QGraphicsItem * parent) 
: DiagramElement(parent)
{
	start = isStart;
	setPos(pos);
	color = Qt::green;
	edgepointShape = Point; 
	lineAngle = 0.0;

	setZValue(EDGEPOINT_Z_VALUE);
	setFlags(ItemIsSelectable|ItemIsMovable);
	setAcceptsHoverEvents(false);
	setAcceptedMouseButtons(Qt::LeftButton);
}

void Edgepoint::moveBy(const QPointF & vector)
{
	QGraphicsItem::moveBy(vector.x(),vector.y());
}

QRectF Edgepoint::boundingRect() const
{
	QTransform matrix;

	switch(edgepointShape)
	{
		case Point:
			return QRectF(-EDGEPOINT_SIZE,-EDGEPOINT_SIZE,2*EDGEPOINT_SIZE,2*EDGEPOINT_SIZE);
		break;
		case ArrowPos:
		case ArrowNeg:
			matrix.rotate(-(180/Pi)*lineAngle);
			return (polygon*matrix).boundingRect(); 
		break;
		default:
			/* nerozpoznany tvar */
			Q_ASSERT(false);
			return QRect();	/* kvuli warningu */
	}
}

QPainterPath Edgepoint::shape() const
{
	QPainterPath path;
	QTransform matrix;

	switch(edgepointShape)
	{
		case Point:
			path.addEllipse(boundingRect());
		break;
		case ArrowPos:
		case ArrowNeg:
			matrix.rotate(-(180/Pi)*lineAngle);
			path.addPolygon(polygon*matrix);
		break;
		default:
			/* nerozpoznany tvar */
			Q_ASSERT(false);
	}

	return path;
}

void Edgepoint::initPolygon()
{
	polygon = QPolygonF();

	QPointF arrowP0;
	QPointF arrowP1;
	QPointF arrowP2;

	if(sin(lineAngle) >= 0)
		lineAngle = (2*Pi) - lineAngle; 
	
	switch(edgepointShape)
	{
		/* ArrowPos: ????----> */
		case ArrowPos:
			arrowP0 = QPointF(10*cos(lineAngle),10*sin(lineAngle));
			arrowP1 = arrowP0 - QPointF(sin(lineAngle + Pi / 3) * ARROW_SIZE,
						cos(lineAngle + Pi / 3) * ARROW_SIZE);
			arrowP2 = arrowP0 - QPointF(sin(lineAngle + Pi - Pi / 3) * ARROW_SIZE,
						cos(lineAngle + Pi - Pi / 3) * ARROW_SIZE);
		break;
		/* ArrowNeg: ????----< */
		case ArrowNeg:
			arrowP0 = QPointF(-10*cos(lineAngle),-10*sin(lineAngle));
			arrowP1 = arrowP0 + QPointF(sin(lineAngle + Pi / 3) * ARROW_SIZE,
						cos(lineAngle + Pi / 3) * ARROW_SIZE);
			arrowP2 = arrowP0 + QPointF(sin(lineAngle + Pi - Pi / 3) * ARROW_SIZE,
						cos(lineAngle + Pi - Pi / 3) * ARROW_SIZE);
		break;
		default:
			/* nerozpoznany tvar */
			Q_ASSERT(false);
	}

	polygon << arrowP0 << arrowP1 << arrowP2;
}

void Edgepoint::updateAngle(double angle)
{
	lineAngle = angle;	
	update();
}

void Edgepoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget)
{
	Q_UNUSED(widget);
	Q_UNUSED(option);
	
	painter->setPen(QPen(Qt::gray,1));
	painter->setBrush(color);

	switch(edgepointShape)
	{
		case Point:
			painter->drawEllipse(boundingRect());
		break;
		case ArrowPos:
		case ArrowNeg:
			painter->save();
			painter->rotate(-(180/Pi)*lineAngle); /* uhel musi byt ve stupnich */
			painter->drawPolygon(polygon);
			painter->restore();
		break;
		default:
			/* nerozpoznany tvar */
			Q_ASSERT(false);
	}
	
/*	test kolizniho tvaru	
 	painter->setPen(QPen(Qt::red,1));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(shape()); */
}

QColor Edgepoint::getColor() const
{
	return color;
}

void Edgepoint::setColor(QColor newColor)
{
	color = newColor;
}

void Edgepoint::setShape(EdgepointShape shape)
{
	edgepointShape = shape;

	if ( (shape == ArrowPos) || (shape == ArrowNeg) )
		initPolygon();
}
