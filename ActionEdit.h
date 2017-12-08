#ifndef ACTION_EDIT_H
#define ACTION_EDIT_H

#include "DrivenEditWidget.h"

class ActionEdit : public DrivenEditWidget
{
	Q_OBJECT
	
	public:
		ActionEdit(QWidget * parent = 0);
		
	private:
		void defineRectangleNode(QPointF pos, int newID);
		void defineEllipseNode(QPointF pos, int newID);
		void displayInfo(int nodeID);
};

#endif
