#ifndef PREDICATE_INFO_TASK_H
#define PREDICATE_INFO_TASK_H

#include "InfoPanel.h"
#include "ui_predicateInfoTask.h"

class DataWidget;

class PredicateInfoTask : public InfoPanel
{
	Q_OBJECT

	public:
		PredicateInfoTask(DataWidget * data, const DataWidget * definition, int predicateID, QWidget * parent = 0);
		void updateContent();

	private slots:
		void on_moveDown_pressed();
		void on_moveUp_pressed();
	
		void on_initRadioButton_toggled(bool checked);
		void on_goalRadioButton_toggled(bool checked);

	private:
		const DataWidget * xmlDefinition;
		QDomElement predicateDef;

		void fillArgumentTable(); 
		void setRadioButtons();
		void moveCurrentRow(RowMoveDirection direction);
		void swapTableItems(QTableWidgetItem * first, QTableWidgetItem * second);
		void swapConnections(int firstArg, int secondArg);

		void setPredicateState(QString newState);

		Ui::PredicateInfoTask ui;
};

#endif
