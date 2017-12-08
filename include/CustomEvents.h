#ifndef CUSTOM_EVENTS_H
#define CUSTOM_EVENTS_H

#include <QEvent>
#include <QVariant>
#include <QString>

/* globalni promenne pro typy eventu */
extern QEvent::Type DiagramLeftClick;
extern QEvent::Type DiagramRightClick;	
extern QEvent::Type NodeLeftClick;
extern QEvent::Type NodeRightClick;
extern QEvent::Type EdgeLeftClick; 
extern QEvent::Type EdgeRightClick;
extern QEvent::Type NodeMoved;
extern QEvent::Type EdgeMoved;
extern QEvent::Type NodeDrag;
extern QEvent::Type EdgeDefined;
extern QEvent::Type NodeReshaped;

class DiagramEvent : public QEvent
{
	public:
	DiagramEvent(Type type, int itemID, const QVariant & data = QVariant() );
	
	int itemID() const;
	QVariant data() const;

	private:
	
	int id;
	QVariant eventData;
};

#endif
