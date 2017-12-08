#ifndef DIAGRAM_WIDGET
#define DIAGRAM_WIDGET

#include <QWidget>
#include <QGraphicsScene>
#include <QHash>
#include <QList>

class Node;
class Edge;

struct NodeStructure;
struct EdgeStructure;

/* parametry pomocne cary pro pridavani hran */
#define TEMP_LINE_COLOR "#00FF00"
#define TEMP_LINE_WIDTH 3
#define TEMP_LINE_Z_VALUE 7

class DiagramWidget: public QGraphicsScene
{
	Q_OBJECT
	public:
		enum DiagramMode {InsertLine = 0,DetectClick = 1,MoveItems = 2};

		DiagramWidget(int width, int height, QWidget * parent = 0);

		void reset();

		void setDiagramMode(DiagramMode mode);

		void setNodeVisible(int nodeID, bool visible);
		void setEdgeVisible(int edgeID, bool visible);
		void addNode(const NodeStructure & node);
		void addEdge(const EdgeStructure & edge);

		void removeNode(int id);
		void removeEdge(int id);

		void updateNode(const NodeStructure & update);
		void translateEdge(int id,bool isStart, QPointF vector);

		void stickEdgeToNode(int edgeID, bool start, int nodeID);
		int collidingNodeID(int edgeID, bool start);

	private:
		Node * nodeAt(const QPointF & pos);
		Edge * edgeAt(const QPointF & pos);

		QString layerColor(QString predicateSet);
		QString stateColor(QString predicateState);

		typedef QHash<int,Node*> NodeContainer;
		typedef QHash<int,Edge*> EdgeContainer;

		int newNodeID();
		int newEdgeID();

		NodeContainer dgwNodes;
		EdgeContainer dgwEdges;
		
		DiagramMode myMode;

		/* cara vykreslovana pri pridavani hrany */
		QGraphicsLineItem * tempLine;

		void leftClick(QPointF pos);
		void rightClick(QPointF pos);

		int collidingNodeID(QPointF coord);
		int collidingEdgeID(QPointF coord);
	protected:
		void mousePressEvent(QGraphicsSceneMouseEvent * event);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
		void mouseMoveEvent(QGraphicsSceneMouseEvent * event); 
};

#endif
