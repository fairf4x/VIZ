#ifndef GRAPH_CLASS
#define GRAPH_CLASS

#include <QList>
#include <QMap>
#include <QSet>
#include <igraph.h>

class GraphClass
{
	public:
		GraphClass(QDomElement diagram);
		~GraphClass();
		
		bool init(char nodeMask, char edgeMask, bool directed);
		bool containsCycle();
		int edgeCnt(int nodeID, igraph_neimode_t mode);
		QString nodeLabel(int nodeID) const;
		int nodeParentID(int nodeID) const;

		void addEdge(int startID,int endID);
		void removeEdge(int startID, int endID);
		
		QStringList pathToRoot(int pathStart) const;
		QList<int> childrenNodes(int parent);
		QSet<int> getDescendants(int parent);
		/* testing */
		void print();
	private:
		QList<int> selectComponentReprezentants();
		QList<int> selectNeighbours(int parent);
		QDomElement graphData;
		/* mapuje id pouzivane v XML na id pouzivane v graph 
		 * key - nodeID, value - vertexID */
		QMap<int,int> idMap;
		igraph_t graph;	
		bool directedHasCycle;
};

#endif
