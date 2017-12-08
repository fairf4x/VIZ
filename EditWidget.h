#ifndef EDIT_WIDGET_H
#define EDIT_WIDGET_H
/* zakladni trida urcujici rozhrani
 * - pro editaci jednotlivych diagramu je treba implementovat nektere funkce odlisne 
 * - funkce ktere jsou implementovany zde jsou pro vsechny diagramy shodne
 * - funkce ktere jsou virtualni se lisi je treba implementovat je v potomcich znovu */

#include <QtXml>
#include <QtGui>

#include "DiagramWidget.h"
#include "DataWidget.h"

#define INVALID_ARGN 0

struct EdgeStructure;

class EditWidget: public QGraphicsView
{
	Q_OBJECT

	public:
		/* Editacni mody EditWidgetu (ovlivnuji operace provadene po stisku tlacitka mysi) */
		enum EditMode {Delete,AddEdge,AddEllipse,AddRect};

		enum ChangeCode {	
					RectNodeAdded = 1,
					EllipseNodeAdded = 2,
					AssocEdgeAdded = 3,
					InherEdgeAdded = 4,

					RectNodeDeleted = 5,
					EllipseNodeDeleted = 6,
					AssocEdgeDeleted = 7,
					InherEdgeDeleted = 8,

					AssocEdgeReconnected = 9,
					InherEdgeReconnected = 10,

					ArgumentOrderChanged = 11,

					NodeClassChanged = 12,
					NodeLabelChanged = 13,
					PredicateSetChanged = 14,
					ObjectStateChanged = 15
				};

		EditWidget(QWidget * parent = 0);
	
		void reset();
		bool wasChanged();
		void setEditMode(EditMode mode);
		const DataWidget * xmlDataPointer();
	
	signals:
	
	void sceneChanged(int changeCode);
	void updateInfoPanel(QWidget * infoWidget);

	public slots:

	void setChanged(bool value); /* nastaveni promenne changed na false - napr. po ulozeni */
	void handleExternChange(int changeCode); /* zmena zpusobena jinde nez v diagramu (property editor) */

	void loadXMLdata(QDomDocument diagramElement);
	

	protected:

	/* trihodnotova logika pro prepojovani hran */
	enum ReconnectionValue {	NoChange,	/* beze zmeny - vrat hranu zpet */
					PosChange,	/* prepis pouze pozici */
					PosAssocChange	/* prepis pozici a zaroven zmen asociaci */
				};

	/* promenna uchovavajici aktualni mod */
	EditMode editMode;

	DataWidget * xmlData;
	DiagramWidget * diagram;

	/* regularni vyraz pro kontrolu uzivatelskeho vstupu */
	QRegExp nameChecker;

	char allowedNodeMask;
	char allowedEdgeMask;

	bool changed;
	
	/* zpracovani eventu */
		bool 	event(QEvent * event);
		void 	handleNodeReshape(int id); /* zajistuje magnetismus pripojenych hran pri zmene tvaru uzlu */
		void 	handleNodeMove(int id, QPointF vector);
		void 	handleNodeDrag(int id, QPointF vector);
		void 	handleEdgeMove(int id, bool isStart, QPointF vector);
		void	defineEdge(int startID, int endID, QPointF startPoint, QPointF endPoint, int newID);
	virtual void 	defineRectangleNode(QPointF pos, int newID) = 0;
	virtual void 	defineEllipseNode(QPointF pos, int newID) = 0;
		void 	deleteNode(int nodeID);
		void 	deleteEdge(int edgeID);
	virtual void 	displayInfo(int nodeID) = 0;

	/* pomocne funkce */
	char determineEdgePurpose(char startType, char endType);	

	/* funkce pro kontrolu hran */
	virtual bool 		verifyEdge(EdgeStructure & edge, int & argNum);
	virtual ReconnectionValue verifyReconnection(EdgeStructure & edge, int newNode, 
					bool startMoved, int & argNum) = 0;
	
	/* funkce pro manipulaci s hranami */
	virtual void makeConnection(EdgeStructure & edge, int & argNum) = 0;
	virtual void changeConnection(EdgeStructure & edge, bool isStart, int toNode, int & argNum) = 0;
	virtual void deleteConnection(int edgeID) = 0; 
};

#endif
