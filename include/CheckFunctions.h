#ifndef CHECK_FUNCTIONS
#define CHECK_FUNCTIONS

#include <QtXml>

class DataWidget;
class GraphClass;

struct NodeStructure;

namespace CheckFn
{
	QHash<int,QString> predicateArguments(const QDomElement & predicate);	
	bool checkPredicateDefinition(const QDomElement & predicate, bool verbose);
	bool checkPredicateInstance(const QDomElement & predicate, const QDomElement & definitionRoot, bool verbose);
	bool checkClassInstance(const QDomElement & instance, const QDomElement & definitionRoot, bool verbose);

	bool checkDefinition(const QDomElement & diagramRoot, bool verbose);
	bool checkDependentDiagram(const QDomElement & diagramRoot, const QDomElement & definitionRoot,char rectType, bool verbose);

	void brokenDiagrams(	const QDomDocument & domain, const QDomElement & definitionRoot,
				const QString & groupTagName, const QString & itemTagName, 
				QStringList & correct, QStringList & incorrect);

	QDomElement selectPredicateDefinition(const QDomElement & definitionRoot, const QDomElement & predInstance);
	int getArgNumber(const QDomElement & predicate, int edgeID);
	int getArgCount(const QDomElement & predicate);
	QStringList classFamily(const QString & className, const GraphClass * structure, const DataWidget * definition);

	bool nodeMatches(const QDomElement & probedNode, const NodeStructure & patternNode);

	int getMatchingNodeID(const QDomElement & root, const NodeStructure & nodeTemplate);
	QList<int> selectMatchingIDList(const QDomElement & root, const NodeStructure & nodeTemplate);
	QList<QDomElement> selectMatchingElementList(const QDomElement & root, const NodeStructure & nodeTemplate);
};

#endif
