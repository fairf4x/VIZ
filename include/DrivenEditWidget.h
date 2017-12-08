#ifndef DRIVEN_EDIT_WIDGET_H
#define DRIVEN_EDIT_WIDGET_H

#include "EditWidget.h"

class DrivenEditWidget: public EditWidget
{
	Q_OBJECT

	public:
		DrivenEditWidget(QWidget * parent = 0): EditWidget(parent){};

		void setDefinition(const DataWidget * definition);
	protected:
		const DataWidget * xmlDefinition;
		
		void makeConnection(EdgeStructure & edge, int & argNum);
		void changeConnection(EdgeStructure & edge, bool isStart, int toNode, int & argNum);
		void deleteConnection(int edgeID);
		bool verifyEdge(EdgeStructure & edge, int & argNum);
		EditWidget::ReconnectionValue verifyReconnection(EdgeStructure & edge, int newNodeID,
									bool startMoved, int & argNum);

	private:
		int findArgPosition(const QString & predicate, const QString & argument,const QHash<int,QString> & instArg);
};

#endif
