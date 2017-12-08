#include "classInfo.h"
#include "NodeStructure.h"
#include "DataWidget.h"
#include <iostream>

using namespace std;

ClassInfo::ClassInfo(DataWidget * data, int classID, QWidget * parent):
InfoPanel(data,classID,parent)
{
	ui.setupUi(this);

	ui.className->setText(subelementTagValue(selectedNode,"label"));
}
