#ifndef VIZ_MAIN_WINDOW_H
#define VIZ_MAIN_WINDOW_H

#include <QMainWindow>
#include "ui_VIZ_mainWindow.h"

struct NodeStructure;
struct EdgeStructure;

class TreeModel;
class Convertor;
class QRegExp;

#define VIZ_RECT_NODE 'R'
#define VIZ_ELLIPSE_NODE 'E'

/* kontext pro pridavani hran */
#define VIZ_LANG_DEF 'L'
#define VIZ_ACTION_DEF 'A'
#define VIZ_TASK_DEF 'T'

class VIZ_mainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		VIZ_mainWindow(QWidget * parent = 0);
	
		void appendToLog(bool prefixed, const QString & text, QColor textColor = Qt::black);
		void crash();
	public slots:
		/* Menu File */
		void on_actionNew_triggered();
		void on_actionOpen_triggered();
		void on_actionSave_triggered();
		void on_actionExit_triggered();

		void on_actionProperties_triggered();
		void on_actionExportDomain_triggered();
		void on_actionExportProblems_triggered();
		
		/* Menu Tools*/
		void on_actionClearLog_triggered();
		void on_actionDomainSummary_triggered();

		/* Toolbar */
		void on_actionAddRectNode_toggled(bool checked);
		void on_actionAddEllipseNode_toggled(bool checked);
		void on_actionAddEdge_toggled(bool checked);
		void on_actionDelete_toggled(bool checked);
		
		void on_createAction_triggered();
		void on_deleteAction_triggered();
		
		void on_createProblem_triggered();
		void on_deleteProblem_triggered();

		void on_actionSavePNG_triggered();
		void on_actionCheckDiagram_triggered();

		/* Other */
		void on_tabWidget_currentChanged(int index);
		
		void on_actionSelector_currentIndexChanged(const QString & text);
		void on_taskSelector_currentIndexChanged(const QString & text);
		
		void on_initCheckBox_toggled(bool checked);
		void on_goalCheckBox_toggled(bool checked);

		void on_definitionEdit_sceneChanged(int changeCode);
		void on_actionEdit_sceneChanged(int changeCode);
		void on_taskEdit_sceneChanged(int changeCode);

		/* sloty souvisejici se zobrazovanim podrobnosti v info panelu */
		void on_definitionEdit_updateInfoPanel(QWidget * infoWidget);
		void on_actionEdit_updateInfoPanel(QWidget * infoWidget);
		void on_taskEdit_updateInfoPanel(QWidget * infoWidget);

		void on_classTreeView_clicked(const QModelIndex & index);

	private:
		bool domainChanged;	/* od vytvoreni/nacteni do prvni zmeny je false */
		QRegExp nameChecker;

		QString recentActionName;
		QString recentTaskName;

		Convertor * dataConvertor;
		QDomDocument domainData;
		
		QDomDocument definitionDocument;
		QDomDocument actionDocument;
		QDomDocument taskDocument;

		QDomNode currentAction; /* prave zobrazovana akce - uzel ktery patri do domainData */
		QDomNode currentTask;	/* prave zobrazovany task */
		
		void updateDocument(QDomElement & rootEl, QDomDocument & updatedDoc);
		QDomDocument createEmptyDomain();
		void initDomain(QDomDocument domainDocument);

		QDomElement findDomainSubelement(const QString & tagName);

		void initDefinition(QDomElement diagramElement);
		void initOperators();
		void initProblems();

		bool askForSave(const QString & text);
		void globalSave();
		void saveDefinition();

		/* parentType = "action"/"task"; parentName = obsah atributu name; sectionName = "operators"/"problems" */
		void saveDiagram(	const QString & parentType, const QString & parentName,
					const QString & sectionName, const QDomDocument & diagramDocument);

		void globalSetEditMode(EditWidget::EditMode mode);

		void registerSubelement(QDomElement sub, QComboBox * listWidget);

		TreeModel * classTreeModel;
		void reloadClassTree();
		void updatePropertyEditor();
		void resetPropertyEditor();
		void centerContents(QGraphicsView * view);

		Ui::MainWindow ui;
};

#endif
