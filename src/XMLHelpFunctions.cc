#include <QtXml>
#include "XMLHelpFunctions.h"
#include "EdgeStructure.h"
#include "NodeStructure.h"
#include "DataWidget.h"	/* kvuli definicim NT_CLASS, atd. */

int XMLh::getIntAttribute(const QDomElement & elem, QString attrName)
{
	Q_ASSERT(!elem.isNull());

	if ( !elem.hasAttribute(attrName) )
	{
		qWarning() << "$XMLh::getIntAttribute : attribute" << attrName << "not found in element:" << elem.tagName();
		return -1;
	}
	
	bool ok;

	int result = elem.attribute(attrName).toInt(&ok,10);

	if (!ok)
	{
		qWarning() << "$XMLh::getIntAttribute : int conversion error";
		return -1;
	}
	
	return result;
}

void XMLh::setIntAttribute(QDomElement elem, const QString & attrName, int newContent)
{
	elem.setAttribute(attrName,newContent);
}

int XMLh::getIntValue(const QDomElement & elem)
{
	bool ok;
	int result = elem.text().toInt(&ok);

	if (!ok)
	{
		qWarning() << "$XMLh::getIntValue : int conversion error";
		return -1;
	}
	
	return result;
}

void XMLh::setIntValue(QDomElement & elem, int value)
{
	QDomDocument owner = elem.ownerDocument();
	
	QDomNode newContent = owner.createTextNode(QString().setNum(value,10));
	QDomNode oldContent = elem.firstChild();

	if (oldContent.isNull())
		elem.appendChild(newContent);
	else
		elem.replaceChild(newContent,elem.firstChild());
}

float XMLh::getFloatValue(const QDomElement & elem)
{
	bool ok;
	float result = elem.text().toFloat(&ok);

	if (!ok)
	{
		qWarning() << "$XMLh::getFloatValue : float conversion error";
		return -1;
	}
	
	return result;
}

QString XMLh::getStrValue(const QDomElement & elem)
{
	Q_ASSERT(!elem.isNull());
	return elem.text().trimmed();
}

void XMLh::setStrValue(QDomElement & elem, const QString value)
{
	QDomDocument owner = elem.ownerDocument();

	QDomNode newContent = owner.createTextNode(value);
	QDomNode oldContent = elem.firstChild();

	if (oldContent.isNull())
		elem.appendChild(newContent);
	else
		elem.replaceChild(newContent,elem.firstChild());
}

QPointF XMLh::getPosValue(const QDomElement & elem)
{
	QDomElement xsubel = elem.firstChildElement("x");
	QDomElement ysubel = elem.firstChildElement("y");
	
	if ( xsubel.isNull() || ysubel.isNull() )
	{
		qWarning() << "$XMLh::getPosValue : incomplete pos element";
		return QPointF(0,0);
	}
	
	float xnum = getFloatValue(xsubel);
	float ynum = getFloatValue(ysubel);

	return QPointF(xnum,ynum);
}

char XMLh::detNodeType(const QDomElement & type)
{
	QString typeText = getStrValue(type);
	
	if (typeText == NT_PREDICATE)
		return NST_PREDICATE;
	
	if (typeText == NT_CLASS)
		return NST_CLASS;
	
	if (typeText == NT_OBJECT)
		return NST_OBJECT;
	
	if (typeText == NT_VARIABLE)
		return NST_VARIABLE;
	
	return NS_UNDEF;
}

char XMLh::detEdgePurpose(const QDomElement & purpose)
{
	Q_ASSERT(!purpose.isNull());
	QString purposeText = getStrValue(purpose);

	if (purposeText == EP_ASSOCIATION)
		return DEP_ASSOCIATION;

	if (purposeText == EP_INHERITANCE)
		return DEP_INHERITANCE;

	/* nerozpoznany typ hrany */
	Q_ASSERT(false);
	return 0;
}

bool XMLh::verifyEdgePurpose(const QDomElement & elem, char purpose)
{
	Q_ASSERT(!elem.isNull());

	char targetPurpose = detEdgePurpose(elem.firstChildElement("purpose"));

	return (targetPurpose == purpose);
}

bool XMLh::verifyNodeType(const QDomElement & elem, char type)
{
	Q_ASSERT(!elem.isNull());

	char targetType = detNodeType(elem.firstChildElement("type"));

	return (targetType == type);
}

QStringList XMLh::getNodeList(QDomElement searchRoot, char wantedType)
{
	QStringList result;
	
	QDomElement actNode = searchRoot.firstChildElement("node");

	while ( !actNode.isNull() )
	{
		QDomElement actNodeType = actNode.firstChildElement("type");
		QDomElement actNodeLabel = actNode.firstChildElement("label");
		
		if ( wantedType == detNodeType(actNodeType) )
			result.append(getStrValue(actNodeLabel));
			
		actNode = actNode.nextSiblingElement("node");
	}	

	return result;
}

QDomElement XMLh::findSubelementAttr(const QDomElement & parent, const QString & tagName, const QString & attrName, 
											const QString & attrValue)
{
	/* kontrola vstupnich parametru */
	Q_ASSERT(!parent.isNull());
	Q_ASSERT(!attrName.isNull() && !attrValue.isNull());
 
	QDomElement currentEl = parent.firstChildElement(tagName);	
	
	QString currentValue;

	while ( !currentEl.isNull() )
	{
		currentValue = currentEl.attribute(attrName);
		if ( !currentValue.isNull() )
		{
			if ( currentValue == attrValue )
				return currentEl;
		}
		else
			qWarning() << "$XMLh::findSubelement : missing expected attribute in subelement";

		currentEl = currentEl.nextSiblingElement(tagName);
	}

	/* v pripade nenalezeni pozadovaneho subelementu vraci null */
	return QDomElement();
}

QDomElement XMLh::findSubelementVal(const QDomElement & parent, const QString & tagName, const QString & textVal)
{
	Q_ASSERT(!parent.isNull());

	QDomElement currentEl = parent.firstChildElement(tagName);

	while (!currentEl.isNull())
	{
		if (getStrValue(currentEl) == textVal)
			return currentEl;		

		currentEl = currentEl.nextSiblingElement(tagName);
	}

	return QDomElement();
}

QDomElement XMLh::addSubelement(QDomElement parent, const QString & tagName, const QString & attrName, const QString & attrValue)
{
	Q_ASSERT(!parent.isNull());

	QDomDocument parentDocument = parent.ownerDocument();

	QDomElement subelement = parentDocument.createElement(tagName);

	/* pridavame pouze tag bez atributu */
	if ( attrName.isNull() && attrValue.isNull() )
		return parent.appendChild(subelement).toElement();
		
	if ( attrName.isNull() || attrValue.isNull() )
	{
		qWarning() << "XMLh::addSubelement : missing parameters";
		return QDomElement();
	}
	
	subelement.setAttribute(attrName,attrValue);
	return parent.appendChild(subelement).toElement();

	/* Pozn. z dokumentace neni uplne jasne co vraci funkce QDomNode::appendChild
	 * - predpokladam, ze vraci prave pridany uzel */
}

QString XMLh::subelementTagValue(QDomElement parent, QString tagName)
{
	Q_ASSERT(!parent.isNull());

	QDomElement nodeData = parent.firstChildElement(tagName);
	
	Q_ASSERT(!nodeData.isNull());
	
	return getStrValue(nodeData);	
}

QList<EdgeDefinition> XMLh::selectEdges(QDomElement parent,char edgeMask)
{
	QList<EdgeDefinition> result;

	QDomElement edge = parent.firstChildElement("edge");

	while ( !edge.isNull() )
	{
		char purpose = detEdgePurpose(edge.firstChildElement("purpose"));
		if ( purpose & edgeMask )
		{
			int startID = getIntAttribute(edge.firstChildElement("start"),"nid");
			int endID = getIntAttribute(edge.firstChildElement("end"),"nid");
			
			result.append(qMakePair(startID,endID));
		}
		
		edge = edge.nextSiblingElement("edge");
	}	
	
	return result;
}

QList<int> XMLh::selectNodeIDList(QDomElement parent, char nodeMask)
{
	QList<int> result;
	
	QDomElement node = parent.firstChildElement("node");

	while ( !node.isNull() )
	{
		char type = detNodeType(node.firstChildElement("type"));
		if ( type & nodeMask )
			result.append(getIntAttribute(node,"id"));
		
		node = node.nextSiblingElement("node");
	}

	return result;
}

QSet<QString> XMLh::selectNodeLabelSet(QDomElement parent, char nodeMask)
{
	QSet<QString> result;
	
	QDomElement node = parent.firstChildElement("node");

	while ( !node.isNull() )
	{
		char type = detNodeType(node.firstChildElement("type"));
		if ( type & nodeMask )
			result.insert(subelementTagValue(node,"label"));
		
		node = node.nextSiblingElement("node");
	}

	return result;
}

QList<int> XMLh::connectedEdges(const QDomElement & node)
{
	QList<int> result;

	Q_ASSERT(node.tagName() == "node");
	QDomElement connections = node.firstChildElement("connections");

	if (connections.isNull())
		return result;

	QDomNode connEl = connections.firstChild();

	while ( !connEl.isNull() )
	{
		result.append(getIntValue(connEl.toElement()));
		
		connEl = connEl.nextSibling();
	}
	
	return result;
}

QMap<int,int> XMLh::definedArguments(const QDomElement & predicate)
{
	Q_ASSERT(predicate.tagName() == "node");
	Q_ASSERT(!predicate.isNull());
	Q_ASSERT(verifyNodeType(predicate,NST_PREDICATE));

	QMap<int,int> result;

	QDomElement connections = predicate.firstChildElement("connections");

	if (connections.isNull())
		return result;

	QDomElement connEl = connections.firstChildElement("starts");

	while ( !connEl.isNull() )
	{
		int edgeID = getIntValue(connEl);
		int argNum = getIntAttribute(connEl,"argn");
		result.insert(argNum,edgeID);
		
		connEl = connEl.nextSiblingElement("starts");
	}
	
	return result;
}

QStringList	XMLh::correspondingNodeContent(QList<int> & idList, QDomElement searchRoot, QString tagName)
{
	QHash<int,QString> idHash;

	QDomElement node = searchRoot.firstChildElement("node");
	while ( !node.isNull() )
	{
		int actID = getIntAttribute(node,"id");
		if ( !idList.contains(actID) )
		{
			node = node.nextSiblingElement("node");
			continue;
		}
		
		QString actValue = subelementTagValue(node,tagName);
		idHash.insert(actID,actValue);

		node = node.nextSiblingElement("node");	
	}

	QStringList result;

	foreach (int id,idList)
		result.append(idHash.value(id));

	return result;	
}

int XMLh::associatedNodeID(const QDomElement & edge, bool start)
{
	Q_ASSERT(!edge.isNull());

	QDomElement edgepoint;
	if (start)
		edgepoint = edge.firstChildElement("start");	
	else
		edgepoint = edge.firstChildElement("end");

	Q_ASSERT(!edgepoint.isNull());

	return getIntAttribute(edgepoint,"nid");	
}
