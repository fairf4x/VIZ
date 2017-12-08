#ifndef EDGEPOINT_H
#define EDGEPOINT_H

#define EDGEPOINT_Z_VALUE 6
#define EDGEPOINT_SIZE 6
#define EDGEPOINT_ARROW_SIZE (EDGEPOINT_SIZE+4)
#define ARROW_SIZE 15

#include "DiagramElement.h"

class Edgepoint: public DiagramElement
{
	public:
		enum EdgepointShape {Point,ArrowPos,ArrowNeg};
		Edgepoint(const QPointF & pos, bool isStart, QGraphicsItem * parent = 0);

		/* funkci moveBy v QGraphicsItem nelze zavolat s jednim parametrem */
		void moveBy(const QPointF & vector);

		QColor getColor() const;
		void setColor(QColor newColor);
		void setShape(EdgepointShape shape);

		QRectF boundingRect() const;
		QPainterPath shape() const;
		void updateAngle(double angle);
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget); 

	private:
		bool start;
		EdgepointShape edgepointShape;
		QColor color;

		QPolygonF polygon;
		double lineAngle;

		void initPolygon();
};

#endif
