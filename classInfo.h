#ifndef CLASS_INFO_H
#define CLASS_INFO_H

#include "InfoPanel.h"
#include "ui_classInfo.h"

class DataWidget;

class ClassInfo : public InfoPanel
{
	Q_OBJECT

	public:
		ClassInfo(DataWidget * data, int classID, QWidget * parent = 0);

	private:

	Ui::ClassInfo ui;	
};

#endif
