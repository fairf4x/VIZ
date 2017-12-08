#ifndef TASK_EDIT_H
#define TASK_EDIT_H

#include "DrivenEditWidget.h"

class TaskEdit : public DrivenEditWidget
{
	Q_OBJECT

	public:
		enum PredicateState { InitState, GoalState, NoState };

		TaskEdit(QWidget * parent = 0);

		void setTaskState(PredicateState state);
		void setPredicatesVisible(bool visible, PredicateState selector);

	private:
		PredicateState taskState;	/* urcuje defaultni zaskrtnuti stavu v dialogu pro pridani predikatu */
		void defineRectangleNode(QPointF pos, int newID);
		void defineEllipseNode(QPointF pos, int newID);
		void displayInfo(int nodeID);
};

#endif
