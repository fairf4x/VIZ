#ifndef OBJECT_INFO_H
#define OBJECT_INFO_H

#include "InfoPanel.h"
#include "ui_objectInfo.h"

class DataWidget;

class ObjectInfo : public InfoPanel
{
	Q_OBJECT

	public:
		ObjectInfo(DataWidget * data, const DataWidget * definition, int objectID, int definitionID, QWidget * parent = 0);

	private slots:

		void on_classComboBox_currentIndexChanged(QString newClass);

	private:
	const DataWidget * xmlDefinition;
	QDomElement objectDef;

	void initComboBox();

	Ui::ObjectInfo ui;	
};
 
#endif
