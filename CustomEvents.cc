#include "CustomEvents.h"

/* zaregistrovani kodu pro specialni eventy */
QEvent::Type DiagramLeftClick 	= (QEvent::Type) QEvent::registerEventType(QEvent::User);
QEvent::Type DiagramRightClick 	= (QEvent::Type) QEvent::registerEventType(QEvent::User + 1);
QEvent::Type NodeLeftClick 	= (QEvent::Type) QEvent::registerEventType(QEvent::User + 2);
QEvent::Type NodeRightClick 	= (QEvent::Type) QEvent::registerEventType(QEvent::User + 3);
QEvent::Type EdgeLeftClick 	= (QEvent::Type) QEvent::registerEventType(QEvent::User + 4);
QEvent::Type EdgeRightClick 	= (QEvent::Type) QEvent::registerEventType(QEvent::User + 5);
QEvent::Type NodeMoved 		= (QEvent::Type) QEvent::registerEventType(QEvent::User + 6);
QEvent::Type EdgeMoved 		= (QEvent::Type) QEvent::registerEventType(QEvent::User + 7);
QEvent::Type NodeDrag 		= (QEvent::Type) QEvent::registerEventType(QEvent::User + 8);
QEvent::Type EdgeDefined 	= (QEvent::Type) QEvent::registerEventType(QEvent::User + 9);
QEvent::Type NodeReshaped	= (QEvent::Type) QEvent::registerEventType(QEvent::User + 10);

DiagramEvent::DiagramEvent( Type type, int itemID,const QVariant & data):QEvent(type)
{
	id = itemID;
	eventData = data;
	setAccepted(false);	/* chceme poslat rodicum */
}

int DiagramEvent::itemID() const
{
	return id;
}

QVariant DiagramEvent::data() const
{
	return eventData;
}
