#ifndef INFO_PANEL_H
#define INFO_PANEL_H

#include <QWidget>
#include <QtGui>
#include <XMLHelpFunctions.h>
#include "DataWidget.h"

enum RowMoveDirection {MoveUp, MoveDown};

class InfoPanel : public QWidget
{
	Q_OBJECT

	public:
		InfoPanel(QWidget * parent = 0):
		QWidget(parent)
		{
			QHBoxLayout * layout = new QHBoxLayout;
			QLabel * hintLabel = new QLabel("Right click on some node.");
			hintLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
			layout->addWidget(hintLabel);
			setLayout(layout);
		}

		InfoPanel(DataWidget * data, int nodeID, QWidget * parent = 0):
		QWidget(parent)
		{
			xmlData = data;
			selectedNode = xmlData->findNode(nodeID);
		}	
		virtual void updateContent(){};

	signals:
	
		void madeChange(int changeCode);

	protected:
	DataWidget * xmlData;
	QDomElement selectedNode;
};

#endif
