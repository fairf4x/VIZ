#ifndef VARIABLE_INFO_H
#define VARIABLE_INFO_H

#include "InfoPanel.h"
#include "ui_variableInfo.h"

class DataWidget;

class VariableInfo : public InfoPanel
{
	Q_OBJECT

	public:
		VariableInfo(DataWidget * data, const DataWidget * definition, int variableID, int definitionID, QWidget * parent = 0);

	private slots:

		void on_classComboBox_currentIndexChanged(QString newClass);

	private:
	const DataWidget * xmlDefinition;
	QDomElement variableDef;

	void initComboBox();

	Ui::VariableInfo ui;	
};
 
#endif
