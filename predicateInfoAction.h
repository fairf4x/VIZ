#ifndef PREDICATE_INFO_ACTION_H
#define PREDICATE_INFO_ACTION_H

#include "InfoPanel.h"
#include "ui_predicateInfoAction.h"

class DataWidget;

class PredicateInfoAction : public InfoPanel
{
	Q_OBJECT

	public:
		PredicateInfoAction(DataWidget * data,const DataWidget * definition, int predicateID, QWidget * parent = 0);
		void updateContent();

	private slots:
		void on_moveDown_pressed();
		void on_moveUp_pressed();
		
		void on_precondCheck_toggled(bool checked);
		void on_effectPosCheck_toggled(bool checked);
		void on_effectNegCheck_toggled(bool checked);

	private:
		const DataWidget * xmlDefinition;
		QDomElement predicateDef;

		void fillArgumentTable(); 
		void setCheckBoxes();
		void moveCurrentRow(RowMoveDirection direction);
		void swapTableItems(QTableWidgetItem * first, QTableWidgetItem * second);
		void swapConnections(int firstArg, int secondArg);

		void includeInSet(QString set);
		void excludeFromSet(QString set);

		Ui::PredicateInfoAction ui;	
};

#endif
