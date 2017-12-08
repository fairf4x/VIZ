#include "DiagramElement.h"

DiagramElement::DiagramElement(QGraphicsItem * parent)
: QGraphicsItem(parent)
{
}

void DiagramElement::stick(const DiagramElement * another)
{
	Q_ASSERT(another != 0);

	/* vytyceni "posouvaci primky" */
	QLineF wayLine(scenePos(),another->pos());
	qDebug() << "wayLine" << scenePos() << another->pos();

	qreal delta = 0;

	while(!collidesWithItem(another))
	{
		this->setPos(wayLine.pointAt(delta));
		delta += 0.1;
		
		if(delta > 1)
		{
			qDebug() << "$DiagramElement::stick : collision should occured";
			break;
		}
	}
}
