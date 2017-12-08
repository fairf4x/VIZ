#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <QtXml>
#include "XMLHelpFunctions.h"

/* node types */
#define NT_CLASS "class"
#define NT_PREDICATE "predicate"
#define NT_VARIABLE "variable"
#define NT_OBJECT "object"

/* edge purposes */
#define EP_ASSOCIATION "association"
#define EP_INHERITANCE "inheritance"

/* predicate set names */
#define PS_PRECOND "precondition"
#define PS_EFFECT_POS "effect+"
#define PS_EFFECT_NEG "effect-"

class DiagramWidget;
class TreeModel;

struct NodeStructure;
struct EdgeStructure;

using namespace XMLh;
/* zakonceni hrany je v domene jednoznacne definovano pomoci ID hrany a informaci, zda se jedna o zacatek */
typedef QPair<int,bool> DataEdgepoint;

class DataWidget
{
	public:
		DataWidget(DiagramWidget * diagram);

		QDomElement diagramRootElement() const;

		void loadData(QDomDocument diagramDocument);

		/* funkce pro ziskavani informaci z XML */
		
		QList<DataEdgepoint>	associatedEdgepoints(int nodeID) const;
		QList<int>		associatedEdges(int nodeID) const;
		QPair<int,int>		associatedNodes(int edgeID) const;

		QString 		getNodeData(int id, QString tagName) const;
		const EdgeStructure 	getEdgeDescription(int id) const;
		QDomElement 		findNode(int id) const;
		QDomElement 		findEdge(int id) const;
		
		char 			readEdgePurpose(int edgeID);
		char 			nodeType(int nodeID);
		
		QSet<QString> 		getValidPredicateSet() const;

		QStringList 		getNodeLabelList(char type) const;
		QSet<QString> 		getNodeLabelSet(char type) const;
		
		int 			getMatchingNodeID(const NodeStructure & nodeTemplate) const;
		QList<int> 		selectMatchingIDList(const NodeStructure & nodeTemplate) const;
		QList<QDomElement> 	selectMatchingElementList(const NodeStructure & nodeTemplate) const;

		/* funkce s propagaci do diagramu */
		void 			refreshNode(int nodeID);

		/* funkce pro upravu dat v XML */
		void 			addDataNode(const NodeStructure & node);
		void 			addDataEdge(const EdgeStructure & edge);

		void 			delDataNode(int id);
		EdgeDefinition 		delDataEdge(int id);

		void 			changeNodePos(int id, const QPointF & vector);
		void 			changeEdgePos(int id, bool isStart,const QPointF & vector); 

		/* funkce pro manipulaci s hranami */
		void 			changeEdgeAssociation(int id, bool isStart, int toNode);
		QDomElement 		disconnectEdgeFromNode(int edgeID, bool start, int nodeID);
		QDomElement 		connectEdgeToNode(int edgeID, bool start, int nodeID);
	
	private:
		QDomDocument 		XMLdata;	
		QDomElement 		diagramData;	/* odkaz na root element zpracovavanych dat */
		DiagramWidget*	 	diagramScene;

		/* obecne funkce pro upravu dat v XML */
		void 		addVector(QDomElement & pos, QPointF vector);
		void		setPosition(QDomElement & pos, QPointF newPos);
		QDomElement	addChildElement(QDomElement & parent, QString childName, QString content);

		void		resetPredicate(QDomElement & elem);

		/* konverze QDomElementu na strukturu DiagramWidgetu */	
		const NodeStructure loadNode(const QDomElement & elem) const;
		const EdgeStructure loadEdge(const QDomElement & elem) const;
};

#endif
