#ifndef NODE_H
#define NODE_H

#ifndef INVALID_ID
#define INVALID_ID -1
#endif

#define NODE_Z_VALUE 3
#define NODE_PADDING 10

#define NODE_LAYER_CNT 3
#define NODE_LAYER_WIDTH 5

#include "DiagramElement.h"
#include <QtGui>

class Node : public DiagramElement
{
public:
	enum NodeShape {Rectangle,Ellipse,Undefined};

	Node();
	Node(int nID, NodeShape nShape, QGraphicsScene * scene = 0, QObject * host = 0);

	bool isValid();
	int getID();

	/* manipulace s barvou */
	const QColor & getColor();
	void setColor(const QColor & color);

	/* manipulace s labelem */
	const QString getLabel();
	void setLabel(const QString & label);

	/* manipulace s vrstvami (tyka se pouze predikatu) */
	void setLayer(const QString & col);
	void clearLayers();

	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget); 

protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);	

private:
	int id;
	QObject * eventTarget;
	QSet<QString> nodeLayers;
	QSize nodeSize;
	QGraphicsSimpleTextItem * nodeLabel;
	NodeShape nodeShape;
	QColor color;

	QPointF dragStartPos;

	void drawLayers(QPainter * painter);
};

#endif
