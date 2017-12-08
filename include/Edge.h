#ifndef EDGE_H
#define EDGE_H

#ifndef INVALID_ID
#define INVALID_ID -1
#endif

#define EDGE_Z_VALUE 5
#define EDGE_LINE_WIDTH 3

/* hodnoty pro rozliseni zacatku a konce hrany */
#define EDGE_START 	true
#define EDGE_END	false

#include <QtGui>
#include <QGraphicsLineItem>

class Node;
class DiagramElement;
class Edgepoint;

class Edge : public QGraphicsLineItem
{
	public:
		enum EdgeOrientation {Forward,Backward,NoOrientation};
		Edge();
		Edge(const QLineF & line, int id, QGraphicsScene * scene = 0, QObject * host = 0);

		bool isValid();
	
		void setPos(const QPointF & newPos,bool isStart);
		void moveBy(const QPointF & vector,bool isStart);
		void stickTo(const DiagramElement * another,bool isStart);

		void setEdgeOrientation(EdgeOrientation orientation);
		void setPointColor(QColor color);

		int getID();

		int connectedNodeID(bool isStart,QList<Node*> nodeList);

		QPainterPath shape() const;
		QRectF boundingRect() const;
	
		/* patri do private - vratit !!*/
		Edgepoint * start;
		Edgepoint * end;

	private:
		int edgeId;
		QObject * eventTarget;
		
		
		/* promenna pro ulozeni zacatku pohybu hrany */
		QPointF startGrabPos;
		QPointF endGrabPos;

		double getAngle();

		bool sceneEventFilter(QGraphicsItem * watched, QEvent * event);
		void handleClickEvent(bool isStart, QPointF pos);
};

#endif
