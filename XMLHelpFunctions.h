#ifndef XML_HELP_FUNCTIONS
#define XML_HELP_FUNCTIONS

#include <QtXml>

/* obecne funkce pro ziskavani dat z XML */
namespace XMLh
{
	/* hrana je definovana pomoci ID dvou uzlu, ktere propojuje <startID,endID> */
	typedef QPair<int,int> EdgeDefinition;

	int	getIntAttribute(const QDomElement & elem, QString attrName);
	void	setIntAttribute(QDomElement elem, const QString & attrName, int newContent);
	int 	getIntValue(const QDomElement & elem); /* vrati celociselnou hodnotu obsazenou v zadanem tagu */
	void	setIntValue(QDomElement & elem, int value);
	float	getFloatValue(const QDomElement & elem); /* vrati realnou hodnotu */
	QString	getStrValue(const QDomElement & elem); /* vrati retezec obsazeny v zadanem tagu */
	void	setStrValue(QDomElement & elem, const QString value);
	QPointF	getPosValue(const QDomElement & elem); /* vrati pozici definovanou podelementy <x> a <y> */

	char	detNodeType(const QDomElement & type);

	char	detEdgePurpose(const QDomElement & purpose);
	
	bool	verifyEdgePurpose(const QDomElement & elem, char purpose);
	bool	verifyNodeType(const QDomElement & elem, char type);
	QStringList getNodeList(QDomElement searchRoot, char wantedType);

	QDomElement findSubelementAttr(const QDomElement & parent, const QString & tagName, const QString & attrName = QString(),
					const QString & attrValue = QString() ); 
	QDomElement findSubelementVal(const QDomElement & parent, const QString & tagName, const QString & textVal);

	QDomElement addSubelement(QDomElement parent, const QString & tagName, const QString & attrName = QString(), 
					const QString & attrValue = QString() );

	QString subelementTagValue(QDomElement parent, QString tagName);
	QList<EdgeDefinition> selectEdges(QDomElement parent, char edgeMask); 	
	QList<int>	selectNodeIDList(QDomElement parent, char nodeMask);
	QSet<QString>	selectNodeLabelSet(QDomElement parent, char nodeMask);
	QList<int>	connectedEdges(const QDomElement & node);
	QMap<int,int>	definedArguments(const QDomElement & predicate);
	QStringList	correspondingNodeContent(QList<int> & idList, QDomElement searchRoot, QString tagName);
	int		associatedNodeID(const QDomElement & edge, bool start);		
};

#endif
