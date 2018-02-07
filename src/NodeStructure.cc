#include "NodeStructure.h"

NodeStructure::NodeStructure()
{
}

NodeStructure::NodeStructure(const NodeStructure & original)
{
	content = original.content;
}

Node::NodeShape NodeStructure::shape() const
{
    char type = content.value(nodeType).toChar().toLatin1();
	switch(type)
	{
		case NST_CLASS:
		case NST_VARIABLE:
		case NST_OBJECT:
			return Node::Rectangle;
		case NST_PREDICATE:
			return Node::Ellipse;
		default:
			return Node::Undefined;
	}
}

QColor NodeStructure::color() const
{
    char type = content.value(nodeType).toChar().toLatin1();
	QVariant state;
	switch(type)
	{
		case NST_CLASS:
			return QColor(NC_CLASS);
		case NST_VARIABLE:
			return QColor(NC_VARIABLE);
		case NST_OBJECT:
			return QColor(NC_OBJECT);
		case NST_PREDICATE:
			/* pokud je to predikat v sekci task */
			state = content.value(nodeState);
			if (!state.isNull())
			{
				if ( state.toString() == NSTS_INIT )
					return QColor(NC_INIT_PREDICATE);
				if ( state.toString() == NSTS_GOAL )
					return QColor(NC_GOAL_PREDICATE);
			}
			else	
				/* predikaty v ostatnich sekcich maji jednotnou barvu */
				return QColor(NC_PREDICATE);
		default:
			qWarning() << "NodeStructure::color : can't determine color - undefined node type";
			return QColor("#FFFFFF");
	}
}

void NodeStructure::setData(ContentType type, QVariant data)
{
	/* test vstupnich dat */
	bool ok = true;
	char typeChar;
	switch(type)
	{
	case nodeID:
		data.toInt(&ok);
		if (!ok)
			qWarning() << "$NodeStructure::setData : invalid ID - conversion to int failed";
	break;		
	case nodeType:
        typeChar = data.toChar().toLatin1();
		if ( !(typeChar == NST_CLASS) &&
		     !(typeChar == NST_PREDICATE) &&
		     !(typeChar == NST_VARIABLE) &&
		     !(typeChar == NST_OBJECT) )
		{
			qWarning() << "$NodeStructure::setData : invalid node type specified";
			ok = false;
		}
	break;
	case nodeLabel:
		if ( !data.canConvert(QVariant::String) )
		{
			qWarning() << "$NodeStructure::setData : invalid label - conversion to QString failed";
			ok = false;
		}
	break;
	case nodePosition:
		if (data.toPointF().isNull())
		{
			qWarning() << "$NodeStructure::setData : invalid position - conversion to QPointF failed";
			ok = false;
		}
	break;
	case nodePredicateSet:
		if ( !data.canConvert(QVariant::StringList) )
		{
			qWarning() << "$NodeStructure::setData : invalid predicate set - conversion to QStringList failed";
			ok = false;
		}
	break;
	case nodeState:
		if ( !data.canConvert(QVariant::String) )
		{
			qWarning() << "$NodeStructure::setData : invalid state - conversion to QString failed";
			ok = false;
		}
	break;
	case nodeClass:
		if ( !data.canConvert(QVariant::String) )
		{
			qWarning() << "$NodeStructure::setData : invalid class - conversion to QString failed";
			ok = false;
		}
	break;
	default:
		/* nerozpoznany typ dat */
		Q_ASSERT(false);
	}

	if (ok)
		content.insert(type,data);	
}

QVariant NodeStructure::readData(ContentType type) const
{
	if ( !content.contains(type) )
		return QVariant();	/* neobsahuje nic takoveho */

	return content.value(type);
}

