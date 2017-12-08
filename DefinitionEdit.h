#ifndef DEFINITION_EDIT_H
#define DEFINITION_EDIT_H

#include "EditWidget.h"

class DefinitionEdit : public EditWidget
{
	Q_OBJECT

	public:
		DefinitionEdit(QWidget * parent = 0);


	private:
		void defineRectangleNode(QPointF pos, int newID);
		void defineEllipseNode(QPointF pos, int newID);

		void renumberPredicateArguments(QDomElement toRenumber);

		void displayInfo(int nodeID);
		void makeConnection(EdgeStructure & edge, int & argNum);
		void changeConnection(EdgeStructure & edge, bool isStart, int toNode, int & argNum);
		void deleteConnection(int edgeID);

		bool verifyEdge(EdgeStructure & edge, int & argNum);
		EditWidget::ReconnectionValue verifyReconnection(EdgeStructure & edge, int newNode,
									bool startMoved, int & argNum);
};
#endif
