#include "Node.h"
#include "CustomEvents.h"
#include <QGraphicsScene>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>

#define INVALID_NODE_ID (-1)

extern QEvent::Type NodeReshaped;

Node::Node()
{
	this->id = INVALID_NODE_ID;
}

Node::Node(int nID, NodeShape nShape, QGraphicsScene * scene, QObject * host)
:DiagramElement(0), id(nID), nodeShape(nShape) 
{
	Q_ASSERT(scene != 0);
	Q_ASSERT(host != 0);

	eventTarget = host;

	scene->addItem(this);
	nodeLabel = 0;

	setZValue(NODE_Z_VALUE);
	setFlags(ItemIsSelectable | ItemIsMovable);
	setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton); 
}

bool Node::isValid()
{
	return !(id == INVALID_NODE_ID);
}

int Node::getID()
{
	return id;
}

QRectF Node::boundingRect() const
{
	QRect result;
	
	int bf = nodeLayers.count()+2; /* bubble factor - o kolik ma byt vetsi aby se vesly vrstvy */
	int diff = bf*NODE_LAYER_WIDTH;

	switch(nodeShape)
	{
		case Ellipse:
			result.setRect	(	-(nodeSize.width()/2)-diff,
						-(nodeSize.height()/2)-diff,
						nodeSize.width()+(2*diff),
						nodeSize.height()+(2*diff)
					);

		break;
		case Rectangle:
			result.setRect	(	-(nodeSize.width()/2),
						-(nodeSize.height()/2),
						nodeSize.width(),
						nodeSize.height()
					);
		break;
		default:
			/* nerozpoznany tvar uzlu */
			Q_ASSERT(false);	
	};

	return result;
}

QPainterPath Node::shape() const
{
	QPainterPath path;

	switch(nodeShape)
	{
		case Ellipse:
			path.addEllipse(boundingRect());
		break;
		case Rectangle:
    			path.addRect(boundingRect());
		break;
		default:
			/* nerozpoznany tvar uzlu */
			Q_ASSERT(false);	
	}

	return path;
}

void Node::drawLayers(QPainter * painter)
{
	/* aby vykreslena cara nepresahovala okraj je treba vychozi polomer nastavit mensi */
	/* o vhodne zvolenou konstantu */
	qreal rx = (boundingRect().width()/2)-2;
	qreal ry = (boundingRect().height()/2)-2;

	painter->setBrush(Qt::NoBrush);

	/* prevod QSet na QList */
	QList<QString> layerList = nodeLayers.toList();

	for(int i=0; i < layerList.count(); ++i)
	{
		painter->setPen(QPen(QColor(layerList[i]),NODE_LAYER_WIDTH));
		
		painter->drawEllipse(	QPointF(0,0),rx,ry);
	
	/* aby doslo k zakryti mezer mezi vrstvami je treba ubrat o neco mene nez NODE_LAYER_WIDTH */
		rx -= (NODE_LAYER_WIDTH-1);
		ry -= (NODE_LAYER_WIDTH-1);
	}
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(widget);
	Q_UNUSED(option);

	painter->setPen(QPen(Qt::gray,1));
	painter->setBrush(color);

	switch(nodeShape)
	{
		case Ellipse:
			painter->drawEllipse(boundingRect());

			if(!nodeLayers.isEmpty())
				drawLayers(painter);
		break;
		case Rectangle:
			painter->drawRect(boundingRect());
		break;
		default:
			/* nerozpoznany tvar uzlu */
			Q_ASSERT(false);	
	}
/* testovani kolizniho tvaru 
	painter->setBrush(Qt::NoBrush);
	painter->setPen(QPen(Qt::red,1));
	painter->drawPath(shape()); */
}


const QColor & Node::getColor()
{
	return color;
}

void Node::setColor(const QColor & color)
{
	this->color = color;
}

const QString Node::getLabel()
{
	return nodeLabel->text(); 
}

void Node::setLabel(const QString & label)
{
	if (nodeLabel == 0)
		nodeLabel = new QGraphicsSimpleTextItem(label,this);
	else
		nodeLabel->setText(label);

	QRectF larect = nodeLabel->boundingRect();
	nodeLabel->setPos(-(larect.width()/2),-(larect.height()/2));

	nodeSize.setWidth((int)larect.width()+2*NODE_PADDING);
	nodeSize.setHeight((int)larect.height()+2*NODE_PADDING);
	update();

	/* mohla se zmenit plocha kterou uzel zabira - posli event */
	DiagramEvent event( NodeReshaped, id);
	QApplication::sendEvent(eventTarget, &event);
}

void Node::setLayer(const QString & col)
{
	QColor test(col);
	Q_ASSERT(test.isValid());
	nodeLayers.insert(col);
}

void Node::clearLayers()
{
	nodeLayers.clear();
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	qDebug() << "$Drag started";
	dragStartPos = pos();
	QGraphicsItem::mousePressEvent(event);
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	/* vypocet vektoru pro posunuti */
	QPointF vector = pos() - dragStartPos;

	if ( !vector.isNull() )
	{
		qDebug() << "$Drag finished";
		DiagramEvent myEvent( NodeMoved, id, QVariant(vector));
		QApplication::sendEvent(eventTarget,&myEvent);
	}

	QGraphicsItem::mouseReleaseEvent(event);
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QPointF vector = event->scenePos() - event->lastScenePos();
	DiagramEvent myEvent( NodeDrag, id, QVariant(vector));
	QApplication::sendEvent(eventTarget,&myEvent);

	QGraphicsItem::mouseMoveEvent(event);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if ( change == ItemPositionChange && scene() )
	{
		QPointF newPos = value.toPointF();
		QRectF rect = scene()->sceneRect();

		if (!rect.contains(newPos))
		{
			newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
			newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
		
			return newPos;
		}
	}

	return QGraphicsItem::itemChange(change,value);
}
