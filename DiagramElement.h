#ifndef DIAGRAMELEMENT_H
#define DIAGRAMELEMENT_H

#include <QtGui>

class DiagramElement : public QGraphicsItem 
{
	public:
		DiagramElement(QGraphicsItem * parent = 0 );

		/* funkce zajistujici "magnetismus" jednotlivych DiagramElementu */
		void stick(const DiagramElement * another);
	protected:

		/* aby slo vyuzit funkci stick, je treba v potomcich prepsat funkci QPainterPath shape() */
		virtual QPainterPath shape() const = 0;
};

#endif
