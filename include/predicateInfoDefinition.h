#ifndef PREDICATE_INFO_DEFINITION_H
#define PREDICATE_INFO_DEFINITION_H

#include "InfoPanel.h"
#include "ui_predicateInfoDefinition.h"

class DataWidget;

class PredicateInfoDefinition : public InfoPanel
{
	Q_OBJECT

	public:
		PredicateInfoDefinition(DataWidget * data, int predicateID, QWidget * parent = 0);
		void updateContent();

	private slots:
		void on_moveDown_pressed();
		void on_moveUp_pressed();

	private:
		void fillArgumentTable(); 
		void moveCurrentRow(RowMoveDirection direction);
		void swapTableItems(QTableWidgetItem * first, QTableWidgetItem * second);
		void swapConnections(int firstArg, int secondArg);

		Ui::PredicateInfoDefinition ui;	
};

#endif
