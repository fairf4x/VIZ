#include <QtGui>

#include "VIZ_mainWindow.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"
#include "treemodel.h"
#include "XMLHelpFunctions.h"
#include "CheckFunctions.h"
#include "igraph.h" /* kvuli definici IGRAPH_(UN)DIRECTED */
#include "GraphClass.h"
#include "CustomEvents.h"
#include "Convertor.h"
#include "EditWidget.h"		/* kvuli definici EditWidget::ChangeCode */
#include "InfoPanel.h"
#include <iostream>
#include <QFileDialog>
#include <QInputDialog>

/* hlavickove soubory dialogu */
#include "actions_variableDialog.h"
#include "actions_predicateDialog.h"
#include "domainDescDialog.h"

#define DEFAULT_FILENAME "new_domain.xml"
#define MAX_NAME_LENGTH 30	/* maximalni delka jmen akci a problemu */

#define IMAGE_MARGIN 20

/* nazvy objektu jednotlivych zalozek - pro rozliseni ktera je aktualni */
#define CLASS_TAB	"classTab"
#define ACTION_TAB	"actionTab"
#define TASK_TAB	"taskTab"

#define TAB_INDEX_DEFINITION	0
#define TAB_INDEX_OPERATORS	1
#define TAB_INDEX_PROBLEMS	2

using namespace XMLh;
using namespace std;
using namespace CheckFn;

VIZ_mainWindow::VIZ_mainWindow(QWidget * parent): QMainWindow(parent)
{
	ui.setupUi(this);

	dataConvertor = new Convertor(this);

	/* pridani akci pro zobrazeni dock widgetu */
	ui.menuTools->addAction(ui.classDock->toggleViewAction());
	ui.menuTools->addAction(ui.propertyEditorDock->toggleViewAction());
	ui.menuTools->addAction(ui.logDock->toggleViewAction());

	/* vytvoreni exkluzivni skupiny pro akce z toolbaru */
	QActionGroup * toolbarGroup = new QActionGroup(this);
	toolbarGroup->addAction(ui.actionAddRectNode);
	toolbarGroup->addAction(ui.actionAddEllipseNode);
	toolbarGroup->addAction(ui.actionAddEdge);
	toolbarGroup->addAction(ui.actionDelete);

	toolbarGroup->setExclusive(true);

	/* zapnuti resetovani pomocnych widgetu */
	connect(ui.actionNew,SIGNAL(triggered()),ui.actionSelector,SLOT(clear()));
	connect(ui.actionNew,SIGNAL(triggered()),ui.taskSelector,SLOT(clear()));

	/* nastaveni defaultnich hodnot */
	classTreeModel = NULL;
	ui.taskEdit->setTaskState(TaskEdit::InitState);	/* dialog pro predikaty ma defaultne zaskrtnuty stav init */
	
	/* editacni mod je defaultne nastaven na pridavani hranatych uzlu */
	ui.actionAddRectNode->setChecked(true);

	/* domena nebyla zmenena */
	domainChanged = false;

	/* inicializace propertyEditoru */
	InfoPanel * infoWidget = new InfoPanel(ui.propertyEditorDock);
	ui.propertyEditorDock->setWidget(infoWidget);
	infoWidget->show();

	/* inicializace prazdne domeny */
	on_actionNew_triggered();

	/* inicializace regularniho vyrazu pro kontrolu vstupu (nazvy operatoru a problemu) */
	nameChecker.setPattern(QString("^([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[-_]){,%1}$").arg(MAX_NAME_LENGTH));
}

void VIZ_mainWindow::appendToLog(const QString & text, QColor textColor)
{
	ui.logTextEdit->setTextColor(textColor);
    ui.logTextEdit->append(text);
}

void VIZ_mainWindow::crash()
{
	/* vytvor crashdump file */
	QFile file("CRASHDUMP");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;
	
	QTextStream dump(&file);
	
	/* zapis do nej obsah logu*/
	dump << ui.logTextEdit->toPlainText();
	file.close();
	/* ukonci program */
	this->close();
}

void VIZ_mainWindow::initDomain(QDomDocument domainDocument)
{
	if ( !domainDocument.isNull() )	
	{
		/* dokument uz obsahuje nejaka data */
		domainData = domainDocument;
	}
	else
	{
		/* inicializace prazdneho dokumentu */
		domainData = createEmptyDomain();
	}

	QDomElement definitionElement = findDomainSubelement("definition");
	initDefinition(definitionElement.firstChildElement("diagram"));

	recentActionName = QString();
	initOperators();

	recentTaskName = QString();
	initProblems();

	resetPropertyEditor();	

	domainChanged = false;
}

QDomDocument VIZ_mainWindow::createEmptyDomain()
{
	QDomDocument result;
	
	QDomElement rootEl = result.createElement("domain");

	result.appendChild(rootEl);

	rootEl.appendChild(result.createElement("definition"));
	rootEl.appendChild(result.createElement("operators"));
	rootEl.appendChild(result.createElement("problems"));

	return result;
}

void VIZ_mainWindow::on_actionNew_triggered()
{
	if ( !askForSave("New domain") )
		return;

	initDomain(QDomDocument());

	ui.actionTab->setEnabled(false);
	ui.taskTab->setEnabled(false);
}

void VIZ_mainWindow::on_actionOpen_triggered()
{
	if ( !askForSave("Open domain") )
		return;

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("Planning domains (*.xml)"));

	if ( fileName.isEmpty() )
		return;

	QFile domainFile(fileName);

	if ( !domainFile.open(QIODevice::ReadOnly) )
	{
		QMessageBox::critical(this,tr("Error"),tr("Can't open file %1").arg(fileName));
		return;
	}

	domainData = dataConvertor->readFromXML(domainFile); 

	initDomain(domainData);

	ui.tabWidget->setCurrentIndex(TAB_INDEX_DEFINITION);
	centerContents(ui.definitionEdit);
}

void VIZ_mainWindow::on_actionSave_triggered()
{
	globalSave();

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), ".", tr("Planning domains (*.xml)"));

	if ( fileName.isEmpty() )
		return;

	QFile file(fileName);

	if ( !file.open(QIODevice::Truncate| QIODevice::WriteOnly | QIODevice::Text) )
	{
		QMessageBox::critical(this,tr("Error"),tr("Can't open file %1").arg(fileName));
		return;
	}	

	dataConvertor->writeToXML(domainData,file);

	file.close();

	domainChanged = false;
}

void VIZ_mainWindow::on_actionExit_triggered()
{
	if ( !askForSave("Open domain") )
		return;

	/* ukonceni programu */
	qApp->quit();
}

void VIZ_mainWindow::on_actionProperties_triggered()
{
	QDomElement domainEl = domainData.documentElement(); 
	Q_ASSERT(!domainEl.isNull());

	DomainDescDialog dialog(domainEl,this);
	domainChanged = dialog.exec();
}

void VIZ_mainWindow::on_actionExportDomain_triggered()
{
	globalSave();

	QString fileName = QFileDialog::getSaveFileName(this, tr("Export domain"), ".", tr("Planning domains (*.pddl)"));

	if ( fileName.isEmpty() )
		return;

	QFile file(fileName);

	if ( !file.open(QIODevice::Truncate|QIODevice::WriteOnly|QIODevice::Text) )
	{
		QMessageBox::critical(this,tr("Error"),tr("Can't open file %1").arg(fileName));
		return;
	}

	dataConvertor->writeDomainToPDDL(domainData,file);

	file.close();
}

void VIZ_mainWindow::on_actionExportProblems_triggered()
{
	globalSave();
 	QString dirName = QFileDialog::getExistingDirectory(this, tr("Export problems"),".",
					QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);

	if ( dirName.isEmpty() )
		return;

	dataConvertor->writeProblemsToPDDL(domainData,dirName);
}

void VIZ_mainWindow::on_actionClearLog_triggered()
{
	ui.logTextEdit->clear();
}

void VIZ_mainWindow::on_actionDomainSummary_triggered()
{
	/* nejdriv vycisti */
	ui.logTextEdit->clear();

	/* uloz zmeny - pro jistotu */
	globalSave();

	QDomElement definitionDiagram = definitionDocument.firstChildElement("diagram");

	/* ziskej seznam trid */
	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeType,NST_CLASS);

	QList<QDomElement> definedClasses = selectMatchingElementList(definitionDiagram,tempNode);

	if (definedClasses.isEmpty())
		qDebug() << "No classes defined.";
	else
	{
		qDebug() << "Defined classes:";

		foreach(QDomElement actClass, definedClasses)
		{
			qDebug() << subelementTagValue(actClass,"label");
		}
	}

	/* ziskej seznam predikatu */
	tempNode.setData(NodeStructure::nodeType,NST_PREDICATE);

	QList<QDomElement> definedPredicates = selectMatchingElementList(definitionDiagram,tempNode);

	int invalidPredCnt = 0;

	QSet<QString> processed;
	if (definedPredicates.isEmpty())
		qDebug() << "No predicates defined.";
	else
	{
		qDebug() << "Defined predicates:";

		foreach(QDomElement actPred, definedPredicates)
		{
			QString predName = subelementTagValue(actPred,"label");

			if (processed.contains(predName))
				continue;		/* pretizene predikaty vypis jen jednou */

			if(checkPredicateDefinition(actPred,false))
			{
				qDebug() << predName; /* obycejny vypis */
				processed.insert(predName);
			}
			else
			{
				qWarning() << predName; /* barevne odliseny vypis */
				++invalidPredCnt;
				processed.insert(predName);
			}
		}
	}

	/* vypis operatory */
	QStringList correctOper;
	QStringList incorrectOper;
	
	brokenDiagrams(domainData,definitionDiagram,"operators","action",correctOper,incorrectOper);

	if (correctOper.size() > 0)
	{
		qDebug() << "Defined operators:";
		foreach(QString op,correctOper)
		{
			qDebug() << op;
		}
	}

	if ( incorrectOper.size() > 0 )
	{
		qDebug() << "Incorrect operators:";
		foreach(QString op,incorrectOper)
		{
			qWarning() << op;
		}
	}

	if ( (correctOper.size() + incorrectOper.size()) == 0)
		qDebug() << "No operators defined.";

	/* vypis problemy */
	QStringList correctProb;
	QStringList incorrectProb;
	
	brokenDiagrams(domainData,definitionDiagram,"problems","task",correctProb,incorrectProb);

	if (correctProb.size() > 0)
	{
		qDebug() << "Defined problems:";
		foreach(QString prob,correctProb)
		{
			qDebug() << prob;
		}
	}

	if ( incorrectProb.size() > 0)
	{
		qDebug() << "Incorrect problems:";

		foreach(QString prob,incorrectProb)
		{
			qWarning() << prob;
		}
	}

	if ((correctProb.size() + incorrectProb.size()) == 0)
		qDebug() << "No problems defined.";

	qDebug() << "---------Summary---------";
	qDebug() << "#classes: " << definedClasses.count();
	
	if (invalidPredCnt > 0)
		qWarning() << "#predicates: " << (processed.count() - invalidPredCnt) << "/" << invalidPredCnt
			   << " (valid/invalid)";
	else
		qDebug() << "#predicates: " << processed.count();

	if (incorrectOper.size() > 0)
		qWarning() << "#operators: " << correctOper.size() << "+" << incorrectOper.count() << "(correct + incorrect)";
       	else
		qDebug() << "#operators: " << correctOper.size();

	if (incorrectProb.size() > 0)
		qWarning() << "#problems: " << correctProb.size() << "+" << incorrectProb.count() << "(correct + incorrect)";
       	else
		qDebug() << "#problems: " << correctProb.size();
}

void VIZ_mainWindow::on_actionAddRectNode_toggled(bool checked)
{
	if (checked)
		globalSetEditMode(EditWidget::AddRect);	
}

void VIZ_mainWindow::on_actionAddEllipseNode_toggled(bool checked)
{
	if (checked)
		globalSetEditMode(EditWidget::AddEllipse);
}

void VIZ_mainWindow::on_actionAddEdge_toggled(bool checked)
{
	if (checked)
		globalSetEditMode(EditWidget::AddEdge);
}

void VIZ_mainWindow::on_actionDelete_toggled(bool checked)
{
	if (checked)
		globalSetEditMode(EditWidget::Delete);
}

void VIZ_mainWindow::on_createAction_triggered()
{
	bool ok;
	QString newOperatorName = QInputDialog::getText(this, tr("Create new operator"),
			tr("Name:"), QLineEdit::Normal,
			QString("operator"), &ok);
	
	if (!ok)
		return;
	
	if (newOperatorName.isEmpty())
	{
		QMessageBox::information(this,tr("VIZ"),tr("Operator name can't be empty."));
		return;
	}

	if ( !nameChecker.exactMatch(newOperatorName) )
	{
		QMessageBox::warning(this,tr("VIZ"),
		tr("Name has wrong format.\n - only letters, digits, \"-\" and \"_\" are allowed\n- max lenght is limited\n - must start with letter"));
		return;	
	}

	/* pokud je jmeno akce nalezeno mezi existujicimi */
	if ( ui.actionSelector->findText(newOperatorName) >= 0)
	{
		QMessageBox::warning(this,tr("VIZ"),tr("Operator name must be unique."));
		return;
	}
	
	QDomElement operatorsElement = findDomainSubelement("operators");
	Q_ASSERT(!operatorsElement.isNull());

	/* vytvoreni tagu pro novou akci */
	QDomElement newActionElement = domainData.createElement("action");	
	newActionElement.setAttribute("name",newOperatorName);

	/* pridani do XML dokumentu */
	operatorsElement.appendChild(newActionElement);
	
	/* zaregistrovani akce
	 * - akce pridana do actionSelectoru
	 * - je pridan subelement <diagram> (chybi-li) */
	registerSubelement(newActionElement,ui.actionSelector);

	/* nastaveni akce jako aktualni */
	int index = ui.actionSelector->findText(newOperatorName);	
	
	/* akce byla prave pridana, takze index > 0 
	 * - provedenim zmeny indexu se vygeneruje 
	 *   signal ktery spusti on_actionSelector_currentIndexChanged */
	ui.actionSelector->setCurrentIndex(index);
	
	ui.actionTab->setEnabled(true);
	ui.deleteAction->setEnabled(true);
	ui.actionCheckDiagram->setEnabled(true);

	domainChanged = true;
}

void VIZ_mainWindow::on_deleteAction_triggered()
{
	QDomElement operatorsElement = findDomainSubelement("operators");
	Q_ASSERT(!operatorsElement.isNull());

	int actionCount = ui.actionSelector->count();
	if ( actionCount < 1 )
		return;

	QDomElement actionElement;
	actionElement = findSubelementAttr(operatorsElement,"action",
					"name",ui.actionSelector->currentText());
	
	operatorsElement.removeChild(actionElement);
	
	recentActionName = QString();

	/* pokud zbyva posledni akce */
	if ( actionCount == 1)
	{
		ui.actionSelector->clear();
		ui.actionTab->setEnabled(false);	
		ui.deleteAction->setEnabled(false);
		ui.actionCheckDiagram->setEnabled(false);
	}
	/* zbyva jeste vice nez 1 akce */
	else
	{
		int index = ui.actionSelector->currentIndex();
		
		/* spusti funkci on_actionSelector_currentIndexChanged */
		ui.actionSelector->removeItem(index);
	}

	domainChanged = true;
}

void VIZ_mainWindow::on_createProblem_triggered()
{
	bool ok;
	QString newTaskName = QInputDialog::getText(this, tr("Create new task"),
			tr("Task name:"), QLineEdit::Normal,
			QString("problem"), &ok);
	
	if (!ok)
		return;

	if (newTaskName.isEmpty())
	{
		QMessageBox::information(this,tr("VIZ"),tr("Task name can't be empty."));
		return;
	}

	if ( !nameChecker.exactMatch(newTaskName) )
	{
		QMessageBox::warning(this,tr("VIZ"),
		tr("Name has wrong format.\n - only letters and digits are allowed\n- max lenght is limited"));
		return;	
	}

	/* pokud je jmeno problemu nalezeno mezi existujicimi */
	if ( ui.taskSelector->findText(newTaskName) >= 0)
	{
		QMessageBox::warning(this,tr("VIZ"),tr("Task name must be unique."));
		return;
	}
	
	QDomElement problemsElement = findDomainSubelement("problems");
	Q_ASSERT(!problemsElement.isNull());

	QDomElement newTaskElement = domainData.createElement("task"); 
	newTaskElement.setAttribute("name",newTaskName);

	problemsElement.appendChild(newTaskElement);

	registerSubelement(newTaskElement,ui.taskSelector);
	
	int index = ui.taskSelector->findText(newTaskName);
	
	ui.taskSelector->setCurrentIndex(index);

	ui.taskTab->setEnabled(true);
	ui.deleteProblem->setEnabled(true);
	ui.actionCheckDiagram->setEnabled(true);
	domainChanged = true;
}

void VIZ_mainWindow::on_deleteProblem_triggered()
{
	QDomElement problemsElement = findDomainSubelement("problems");

	int taskCount = ui.taskSelector->count();

	/* neni co smazat */
	if ( taskCount < 1 )
		return;
	
	QDomElement taskElement;
	taskElement = findSubelementAttr(problemsElement,"task","name",ui.taskSelector->currentText());
	
	problemsElement.removeChild(taskElement);
	
	recentTaskName = QString();
	
	/* smazany task je posledni */
	if ( taskCount == 1 )
	{
		ui.taskSelector->clear();
		ui.taskTab->setEnabled(false);
		ui.deleteProblem->setEnabled(false);
		ui.actionCheckDiagram->setEnabled(false);
	}
	else
	{
		int index = ui.taskSelector->currentIndex();
		
		/* spusti funkci on_taskSelector_currentIndexChanged */
		ui.taskSelector->removeItem(index);	
	}
	domainChanged = true;
}

void VIZ_mainWindow::on_actionSavePNG_triggered()
{
	qDebug() << "Saving Image";
	/* zjisti jmeno souboru ktery se bude ukladat */
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images (*.png)")); 

	if ( fileName.isEmpty() )
		return;

	/* zjisti ktery diagram je aktualni */
	int activeTab = ui.tabWidget->currentIndex();

	EditWidget * activeDiagram;

	switch(activeTab)
	{
		case TAB_INDEX_DEFINITION:
			activeDiagram = ui.definitionEdit;
		break;
		case TAB_INDEX_OPERATORS:
			activeDiagram = ui.actionEdit;
		break;
		case TAB_INDEX_PROBLEMS:
			activeDiagram = ui.taskEdit;
		break;
		default:
			activeDiagram = ui.definitionEdit;
	}

	QGraphicsScene * diagramScene = activeDiagram->scene();

	QRectF boundingRect = diagramScene->itemsBoundingRect();
	boundingRect.setWidth(boundingRect.width() + 1);
	boundingRect.setHeight(boundingRect.height() + 1);

	QSizeF diagramSize(boundingRect.width()+2*IMAGE_MARGIN,boundingRect.height()+2*IMAGE_MARGIN);
	QImage diagramImage(diagramSize.toSize(),QImage::Format_RGB32);
	/* aby bila bila byla */
	diagramImage.fill(qRgb(255,255,255));

	QPainter diagramPainter(&diagramImage);
	
	const QRectF targetRect(IMAGE_MARGIN,IMAGE_MARGIN,boundingRect.width(),boundingRect.height());
	diagramScene->render(&diagramPainter,targetRect,boundingRect,Qt::KeepAspectRatio);

	diagramImage.save(fileName,"PNG",5);
}

void VIZ_mainWindow::on_actionCheckDiagram_triggered()
{
	ui.logTextEdit->clear();

	int currentTab = ui.tabWidget->currentIndex();

	QDomElement definitionDiagram = definitionDocument.firstChildElement("diagram"); 
	if (currentTab == TAB_INDEX_DEFINITION)
	{
		qDebug() << "Checking language definition.";
		checkDefinition(definitionDiagram,true);
		return;
	}

	if (currentTab == TAB_INDEX_OPERATORS)
	{
		qDebug() << "Checking operator" << recentActionName << ".";
		QDomElement actionDiagram = actionDocument.firstChildElement("diagram");
		checkDependentDiagram(actionDiagram,definitionDiagram,NST_VARIABLE,true);
		return;
	}
	
	if (currentTab == TAB_INDEX_PROBLEMS)
	{
		qDebug() << "Checking problem" << recentTaskName << ".";
		QDomElement taskDiagram = taskDocument.firstChildElement("diagram");
		checkDependentDiagram(taskDiagram,definitionDiagram,NST_OBJECT,true);
		return;
	}

	Q_ASSERT(false);
}

void VIZ_mainWindow::on_tabWidget_currentChanged(int index)
{
	switch(index)
	{
		case TAB_INDEX_DEFINITION:
			/* tlacitka */
			ui.createAction->setEnabled(false);
			ui.deleteAction->setEnabled(false);
			ui.createProblem->setEnabled(false);
			ui.deleteProblem->setEnabled(false);

			ui.actionCheckDiagram->setEnabled(true);
			/* nastaveni pohledu */
			centerContents(ui.definitionEdit);
		break;
		case TAB_INDEX_OPERATORS:
			/* tlacitka */
			ui.createAction->setEnabled(true);
			/* povoli mazani akci a kontrolovani jejich diagramu 
			 * pouze kdyz nejake jsou */
			ui.deleteAction->setEnabled(ui.actionSelector->count() > 0);
			ui.actionCheckDiagram->setEnabled(ui.actionSelector->count() > 0);

			ui.createProblem->setEnabled(false);
			ui.deleteProblem->setEnabled(false);
			/* nastaveni pohledu */
			centerContents(ui.actionEdit);
		break;
		case TAB_INDEX_PROBLEMS:
			/* tlacitka */
			ui.createAction->setEnabled(false);
			ui.deleteAction->setEnabled(false);
			ui.createProblem->setEnabled(true);
			
			/* povolot mazani problemu a kontrolu diagramu pokud nejake jsou */
			ui.deleteProblem->setEnabled(ui.taskSelector->count() > 0);
			ui.actionCheckDiagram->setEnabled(ui.taskSelector->count() > 0);
			/* nastaveni pohledu */
			centerContents(ui.taskEdit);
		break;
		default:
			/* neplatny index */
			Q_ASSERT(false);
	}
	
	/* ulozeni diagramu ze vsech tabu jejichz obsah byl zmenen */
	if ( ui.definitionEdit->wasChanged() )
	{
		saveDefinition();
		ui.definitionEdit->setChanged(false);
	}
	
	if ( ui.actionEdit->wasChanged() )
	{
		saveDiagram("action",ui.actionSelector->currentText(),"operators",actionDocument);
		ui.actionEdit->setChanged(false);
	}
	
	if ( ui.taskEdit->wasChanged() )
	{
		saveDiagram("task",ui.taskSelector->currentText(),"problems",taskDocument);
		ui.taskEdit->setChanged(false);
	}

	resetPropertyEditor();
}

/* funkce pro nacteni novych dat do promennych operatorDocument nebo problemsDocument */
void VIZ_mainWindow::updateDocument(QDomElement & rootEl, QDomDocument & updatedDoc)
{	
	if ( !updatedDoc.isNull() )
	{
		updatedDoc.clear();
		qDebug() << "$VIZ_mainWindow::updateDocument : clearing old data";
	}

	if ( rootEl.isNull() )
	{
		qDebug() << "$VIZ_mainWindow::updateDocument : rootEl is empty";
		return;
	}

	QDomElement targetDiagram = rootEl.firstChildElement("diagram");
	
	QDomNode docDiagram;
	
	if ( targetDiagram.isNull() )
	{
		/* pokud diagram neexistuje, tak ho vytvor - v centralnim dokumentu i v updatedDoc */
		qDebug() << "$VIZ_mainWindow::updateDocument : creating new diagram element";
		targetDiagram = domainData.createElement("diagram");
		rootEl.appendChild(targetDiagram);

		docDiagram = updatedDoc.createElement("diagram");
	}
	else
	{
		/* pokud diagram existuje v centralnim dokumentu - importuje ho */
		docDiagram = updatedDoc.importNode(targetDiagram,true);
	}
	
	/* pridej diagram jako root do updatedDoc */
	updatedDoc.appendChild(docDiagram);
}

void VIZ_mainWindow::on_actionSelector_currentIndexChanged(const QString & text)
{
	if (text.isNull())
	{
		qDebug() << "$on_actionSelector_currentIndexChanged : action with empty name selected";
		ui.actionEdit->reset();
		actionDocument.clear();
		actionDocument.appendChild(actionDocument.createElement("diagram")); /* vytvoreni prazdneho diagramu */
		return;		/* 1. pri resetovani - napr. po provedeni ui.actionSelector->clear() 
				 * 2. pri smazani posledni akce */
	}

	/* uloz predchozi akci (pokud je specifikovana) */
	if ( !recentActionName.isEmpty() )
		saveDiagram("action",recentActionName,"operators",actionDocument);
	
	QDomElement operatorsElement = findDomainSubelement("operators");
	QDomElement demandedAction = findSubelementAttr(operatorsElement,"action","name",text);
	
	/* nahraj obsah diagramu do dokumentu s aktualni akci */
	updateDocument(demandedAction,actionDocument);

	/* nahraj ji do actionEditu */
	qDebug() << "Loading operator:" << text;
	ui.actionEdit->loadXMLdata(actionDocument);
	
	/* updatuj recentActionName */
	recentActionName = text;

	/* nastav pohled */
	centerContents(ui.actionEdit);

	resetPropertyEditor();
}

void VIZ_mainWindow::on_taskSelector_currentIndexChanged(const QString & text)
{
	if ( text.isNull() )
	{
		ui.taskEdit->reset();
		taskDocument.clear();
		taskDocument.appendChild(taskDocument.createElement("diagram"));
		return;		/* 1. po resetovani - ui.taskSelector.clear() 
				 * 2. po smazani posledniho problemu */
	}
	
	if ( !recentTaskName.isEmpty() )
		saveDiagram("task",recentTaskName,"problems",taskDocument);

	QDomElement problemsElement = findDomainSubelement("problems");
	QDomElement demandedTask = findSubelementAttr(problemsElement,"task","name",text);

	updateDocument(demandedTask,taskDocument);

	/* nahraj ho do taskEditu */
	qDebug() << "Loading problem:" << text;
	ui.taskEdit->loadXMLdata(taskDocument);
	
	/* updatuj recentTaskName */
	recentTaskName = text;

	/* nastav pohled */
	centerContents(ui.taskEdit);

	resetPropertyEditor();
}

void VIZ_mainWindow::on_initCheckBox_toggled(bool checked)
{
	if (checked)
	{
		ui.taskEdit->setTaskState(TaskEdit::InitState);
		ui.taskEdit->setPredicatesVisible(true,TaskEdit::InitState);
		ui.actionAddEllipseNode->setEnabled(true);
	}
	else
	{
		ui.taskEdit->setPredicatesVisible(false,TaskEdit::InitState);

		if ( ui.goalCheckBox->isChecked() )
			ui.taskEdit->setTaskState(TaskEdit::GoalState);
		else
		{
			/* znemozneni pridani predikatu */
			ui.actionAddEllipseNode->setEnabled(false);
			ui.taskEdit->setTaskState(TaskEdit::NoState);
		}
	}
}

void VIZ_mainWindow::on_goalCheckBox_toggled(bool checked)
{
	if (checked)
	{
		ui.taskEdit->setTaskState(TaskEdit::GoalState);
		ui.taskEdit->setPredicatesVisible(true,TaskEdit::GoalState);
		ui.actionAddEllipseNode->setEnabled(true);
	}
	else
	{
		ui.taskEdit->setPredicatesVisible(false,TaskEdit::GoalState);

		if ( ui.initCheckBox->isChecked() )
			ui.taskEdit->setTaskState(TaskEdit::InitState);
		else
		{
			/* znemozneni pridani predikatu */
			ui.actionAddEllipseNode->setEnabled(false);
			ui.taskEdit->setTaskState(TaskEdit::NoState);
		}
	}
}

void VIZ_mainWindow::on_definitionEdit_sceneChanged(int changeCode)
{
	qDebug() << "$VIZ_mainWindow::on_definitionEdit_sceneChanged";

	bool checkOperators = false;
	bool checkProblems = false;

	switch(changeCode)
	{	
		case EditWidget::RectNodeAdded:
			qDebug() << "Class defined.";
			reloadClassTree();
		break;
		case EditWidget::EllipseNodeAdded:
			qDebug() << "Predicate defined.";
		break;
		case EditWidget::AssocEdgeAdded:
			qDebug() << "Predicate argument defined.";
			updatePropertyEditor();
		break;
		case EditWidget::RectNodeDeleted:
			qDebug() << "Class deleted.";
			if (ui.actionSelector->count() > 0)
				checkOperators = true;
			if (ui.taskSelector->count() > 0)
				checkProblems = true;
			reloadClassTree();
			resetPropertyEditor();
		break;
		case EditWidget::EllipseNodeDeleted:
			qDebug() << "Predicate deleted.";
			if (ui.actionSelector->count() > 0)
				checkOperators = true;
			if (ui.taskSelector->count() > 0)
				checkProblems = true;
			resetPropertyEditor();
		break;
		case EditWidget::AssocEdgeDeleted:
			qDebug() << "Predicate argument removed.";
			qWarning() << "Domain should be checked.";
			/* kontrola se zde neprovadi automaticky kvuli efektivite
			 * - v pripade odebrani uzlu s pripojenymi hranami 
			 *   by se zbytecne kontrolovalo pro kazdou hranu zvlast */
			updatePropertyEditor();
		break;
		case EditWidget::AssocEdgeReconnected:
			qDebug() << "Predicate argument redefined.";
			if (ui.actionSelector->count() > 0)
				checkOperators = true;
			if (ui.taskSelector->count() > 0)
				checkProblems = true;
			updatePropertyEditor();
		break;
		case EditWidget::InherEdgeAdded:
			qDebug() << "Class inheritance defined.";
			reloadClassTree();
		break;
		case EditWidget::InherEdgeDeleted:
			qDebug() << "Class inheritance removed.";
			qWarning() << "Domain should be checked.";
			/* kontrola se zde neprovadi automaticky kvuli efektivite
			 * - v pripade odebrani uzlu s pripojenymi hranami 
			 *   by se zbytecne kontrolovalo pro kazdou hranu zvlast */
			reloadClassTree();
		break;
		case EditWidget::InherEdgeReconnected:
			qDebug() << "Class inheritance redefined.";
			if (ui.actionSelector->count() > 0)
				checkOperators = true;
			if (ui.taskSelector->count() > 0)
				checkProblems = true;
			reloadClassTree();
		break;
		case EditWidget::ArgumentOrderChanged:
			qDebug() << "Argument order changed.";
			if (ui.actionSelector->count() > 0)
				checkOperators = true;
			if (ui.taskSelector->count() > 0)
				checkProblems = true;
		default:
			qDebug() << "$changeCode =" << changeCode;
	}

	if (checkOperators)
	{
		QStringList correctOper;
		QStringList incorrectOper;

		QDomElement definitionDiagram = definitionDocument.firstChildElement("diagram");
		Q_ASSERT(!definitionDiagram.isNull());

		brokenDiagrams(domainData,definitionDiagram,"operators","action",correctOper,incorrectOper);

		if (incorrectOper.count() > 0)
		{
			qWarning() << "Following operators are now inconsistent:";
			foreach(QString op,incorrectOper)
				qWarning() << op;
		}
	}

	if (checkProblems)
	{
		QStringList correctProb;
		QStringList incorrectProb;

		QDomElement definitionDiagram = definitionDocument.firstChildElement("diagram");
		Q_ASSERT(!definitionDiagram.isNull());

		brokenDiagrams(domainData,definitionDiagram,"problems","task",correctProb,incorrectProb);

		if (incorrectProb.count() > 0)
		{
			qWarning() << "Following problems are now inconsistent:";
			foreach(QString prob,incorrectProb)
				qWarning() << prob;
		}
	}
	
	domainChanged = true;
}

void VIZ_mainWindow::on_actionEdit_sceneChanged(int changeCode)
{
	qDebug() << "$VIZ_mainWindow::on_actionEdit_sceneChanged";

	bool runCheck = false;
	switch(changeCode)
	{
		case EditWidget::RectNodeAdded:
			qDebug() << "Variable defined.";
		break;
		case EditWidget::EllipseNodeAdded:
			qDebug() << "Predicate added.";
		break;
		case EditWidget::AssocEdgeAdded:
			qDebug() << "Association defined.";
			updatePropertyEditor();
		break;
		case EditWidget::RectNodeDeleted:
			qDebug() << "Variable deleted.";
			resetPropertyEditor();
		break;
		case EditWidget::EllipseNodeDeleted:
			qDebug() << "Predicate deleted.";
			resetPropertyEditor();
		break;
		case EditWidget::AssocEdgeDeleted:
			qDebug() << "Association deleted.";
			updatePropertyEditor();
		break;
		case EditWidget::AssocEdgeReconnected:
			qDebug() << "Association changed.";
			updatePropertyEditor();
		break;
		case EditWidget::ArgumentOrderChanged:
			runCheck = true;
		break;
		default:
			qDebug() << "$changeCode =" << changeCode;
	}

	if (runCheck)
	{
		QDomElement definitionDiagram = definitionDocument.firstChildElement("diagram");
		Q_ASSERT(!definitionDiagram.isNull());

		QDomElement actionDiagram = actionDocument.firstChildElement("diagram");
		Q_ASSERT(!actionDiagram.isNull());

		if( !checkDependentDiagram(actionDiagram,definitionDiagram,NST_VARIABLE,false) )
			qWarning() << "Predicate is inconsistent.";
	}	
	
	domainChanged = true;
}

void VIZ_mainWindow::on_taskEdit_sceneChanged(int changeCode)
{
	qDebug() << "$VIZ_mainWindow::on_taskEdit_sceneChanged";

	bool runCheck = false;
	switch(changeCode)
	{
		case EditWidget::RectNodeAdded:
			qDebug() << "Object defined.";
		break;
		case EditWidget::EllipseNodeAdded:
			qDebug() << "Predicate added.";
		break;
		case EditWidget::AssocEdgeAdded:
			qDebug() << "Association defined.";
			updatePropertyEditor();
		break;
		case EditWidget::RectNodeDeleted:
			qDebug() << "Object deleted.";
			resetPropertyEditor();
		break;
		case EditWidget::EllipseNodeDeleted:
			qDebug() << "Predicate deleted.";
			resetPropertyEditor();
		break;
		case EditWidget::AssocEdgeDeleted:
			qDebug() << "Association deleted.";
			updatePropertyEditor();
		break;
		case EditWidget::AssocEdgeReconnected:
			qDebug() << "Association changed.";
			updatePropertyEditor();
		break;
		case EditWidget::ArgumentOrderChanged:
			runCheck = true;
		break;
		default:
			qDebug() << "$changeCode =" << changeCode;
	}

	if (runCheck)
	{
		QDomElement definitionDiagram = definitionDocument.firstChildElement("diagram");
		Q_ASSERT(!definitionDiagram.isNull());

		QDomElement taskDiagram = taskDocument.firstChildElement("diagram");
		Q_ASSERT(!taskDiagram.isNull());

		if( !checkDependentDiagram(taskDiagram,definitionDiagram,NST_VARIABLE,false) )
			qWarning() << "Predicate is inconsistent.";
	}
	domainChanged = true;
}

void VIZ_mainWindow::on_definitionEdit_updateInfoPanel(QWidget * infoWidget)
{
	QWidget * oldWidget = ui.propertyEditorDock->widget();
	ui.propertyEditorDock->setWidget(infoWidget);
	infoWidget->setParent(ui.propertyEditorDock);
	infoWidget->show();

	if (oldWidget != NULL)
		delete oldWidget;
	else
		qWarning() << "$VIZ_mainWindow::on_definitionEdit_updateInfoPanel : oldWidget == NULL";
}

void VIZ_mainWindow::on_actionEdit_updateInfoPanel(QWidget * infoWidget)
{
	QWidget * oldWidget = ui.propertyEditorDock->widget();
	ui.propertyEditorDock->setWidget(infoWidget);
	infoWidget->setParent(ui.propertyEditorDock);
	infoWidget->show();

	delete oldWidget;
}

void VIZ_mainWindow::on_taskEdit_updateInfoPanel(QWidget * infoWidget)
{
	QWidget * oldWidget = ui.propertyEditorDock->widget();
	ui.propertyEditorDock->setWidget(infoWidget);
	infoWidget->setParent(ui.propertyEditorDock);
	infoWidget->show();

	delete oldWidget;
}

void VIZ_mainWindow::on_classTreeView_clicked(const QModelIndex & index)
{
	int clicked = ui.classTreeView->model()->data(index,Qt::UserRole).toInt();
	qDebug() << "$VIZ_mainWindow::on_classTreeView_clicked : ID =" << clicked;
}

QDomElement VIZ_mainWindow::findDomainSubelement(const QString & tagName)
{
	QDomElement domainEl = domainData.documentElement();
	
	return domainEl.firstChildElement(tagName);
}

void VIZ_mainWindow::initDefinition(QDomElement diagramElement)
{
	/* zahozeni puvodniho obsahu */
	definitionDocument.clear();

	/* diagramElement muze byt prazdny pri on_actionNew_triggered */
	if ( diagramElement.isNull() )
	{
		/* vytvoreni prazdneho diagramu */
		qDebug() << "$VIZ_mainWindow::initDefinition : creating empty diagram in definitionDocument";
		definitionDocument.appendChild(definitionDocument.createElement("diagram"));
	}
	else
	{
		/* vytvoreni diagramu s obsahem */
		qDebug() << "$VIZ_mainWindow::initDefinition : loading diagram to definitionDocument";
		definitionDocument.appendChild(definitionDocument.importNode(diagramElement,true));
	}

	ui.definitionEdit->loadXMLdata(definitionDocument);	

	/* propagace definice do ostatnich EditWidgetu (ActionEdit a TaskEdit) */
	ui.actionEdit->setDefinition(ui.definitionEdit->xmlDataPointer());
	ui.taskEdit->setDefinition(ui.definitionEdit->xmlDataPointer());

	reloadClassTree();

	ui.definitionEdit->setChanged(false);
}

void VIZ_mainWindow::initOperators()
{
	actionDocument.clear();
	ui.actionSelector->clear();

	QDomElement operatorsElement = findDomainSubelement("operators");
	
	QDomElement actionEl = operatorsElement.firstChildElement("action");
	
	/* zaregistrovani vsech akci (pridani do actionSelectoru) */
	while ( !actionEl.isNull() )
	{
		registerSubelement(actionEl,ui.actionSelector);
		
		actionEl = actionEl.nextSiblingElement("action");			
	}

	/* nastav aktualni akci jako posledni zobrazenou */
	recentActionName = ui.actionSelector->currentText();
		
	if ( recentActionName.isNull() )
	{
		/* zadna akce nebyla zaregistrovana */
		qDebug() << "No operator defined in this domain.";
		ui.actionTab->setEnabled(false);
	}
	else
	{
		qDebug() << "Count of defined operators in this domain:" << ui.actionSelector->count();
		/* zapni actionTab (uz je co editovat) */
		ui.actionTab->setEnabled(true);
	}
}

void VIZ_mainWindow::initProblems()
{
	taskDocument.clear();
	ui.taskSelector->clear();

	QDomElement problemsElement = findDomainSubelement("problems");
	
	QDomElement taskEl = problemsElement.firstChildElement("task");
	
	while ( !taskEl.isNull() )
	{
		registerSubelement(taskEl,ui.taskSelector);
		
		taskEl = taskEl.nextSiblingElement("task");
	}

	recentTaskName = ui.taskSelector->currentText();

	if ( recentTaskName.isNull() )
	{
		/* zadny task nebyl registrovan */
		qDebug() << "No problem defined in this domain";
		ui.taskTab->setEnabled(false);
	}
	else
	{
		qDebug() << "Count of defined problems in this domain:" << ui.taskSelector->count();
		ui.taskTab->setEnabled(true);
	}
}
bool VIZ_mainWindow::askForSave(const QString & text)
{
	if ( domainChanged )
	{

		QMessageBox::StandardButton answer = QMessageBox::question(this,text,tr("Do you want to save current domain?"),
				QMessageBox::Save|QMessageBox::No|QMessageBox::Cancel,QMessageBox::Save);
		
		if ( answer == QMessageBox::Save )
			on_actionSave_triggered();

		if ( answer == QMessageBox::Cancel)
			return false;
	}

	/* akce ktere dotaz predchazel muze pokracovat */
	return true;
}

void VIZ_mainWindow::globalSave()
{
	/* ulozeni zmen */
	if ( ui.definitionEdit->wasChanged() )
		saveDefinition();

	if ( ui.actionEdit->wasChanged() )
	{
		saveDiagram("action",ui.actionSelector->currentText(),"operators",actionDocument);
		ui.actionEdit->setChanged(false);
	}
	
	if ( ui.taskEdit->wasChanged() )
	{
		saveDiagram("task",ui.taskSelector->currentText(),"problems",taskDocument);
		ui.taskEdit->setChanged(false);
	}
}

void VIZ_mainWindow::saveDefinition()
{
	/* nalezeni elementu v centralnim dokumentu */
	QDomElement definitionElement = findDomainSubelement("definition");	
	Q_ASSERT(!definitionElement.isNull());

	/* nalezeni diagramu v pracovnim dokumentu s definicemi */
	QDomNode definitionDiagram = definitionDocument.firstChildElement("diagram");
	Q_ASSERT(!definitionDiagram.isNull());
	
	/* prevod do podoby pouzitelne v domainData */
	QDomNode newDiagram = domainData.importNode(definitionDiagram,true);
	
	/* nalezeni diagramu pro definice v domainData (bude prepsan) */
	QDomNode oldDiagram = definitionElement.firstChildElement("diagram");

	if ( oldDiagram.isNull() )
		oldDiagram = addSubelement(definitionElement,"diagram");

	/* prepsani diagramu */
	definitionElement.replaceChild(newDiagram,oldDiagram);

	/* vyresetovani promenne changed v definitionEditu */
	ui.definitionEdit->setChanged(false);
}

void VIZ_mainWindow::saveDiagram(	const QString & parentType, const QString & parentName,
					const QString & sectionName,const QDomDocument & diagramDocument)
{

	QDomNode savedDiagram = diagramDocument.firstChildElement("diagram");
	Q_ASSERT(!savedDiagram.isNull());

	/* prevod do podoby pouzitelne v domainData */
	QDomNode newDiagram = domainData.importNode(savedDiagram,true);

	/* nalezeni rodice diagramu v domainData */
	QDomElement sectionElement = findDomainSubelement(sectionName);
	QDomElement parentElement = findSubelementAttr(sectionElement,parentType,"name",parentName);

	/* nalezeni diagramu ktery bude prepsan */
	QDomNode oldDiagram = parentElement.firstChildElement("diagram");

	if ( oldDiagram.isNull() )
		oldDiagram = addSubelement(parentElement,"diagram");

	parentElement.replaceChild(newDiagram,oldDiagram);
}

void VIZ_mainWindow::globalSetEditMode(EditWidget::EditMode mode)
{
	ui.definitionEdit->setEditMode(mode);
	ui.actionEdit->setEditMode(mode);
	ui.taskEdit->setEditMode(mode);
}

void VIZ_mainWindow::registerSubelement(QDomElement sub, QComboBox * listWidget)
{
	Q_ASSERT(!sub.isNull());

	QString subName = sub.attribute("name");
	if ( !subName.isNull() )
	{
		/* pokud tam jeste neni */
		if ( listWidget->findText(subName) < 0 )
		{
			/* pokud chybi subelement <diagram> tak ho pridej */
			if ( sub.firstChildElement("diagram").isNull() )
				addSubelement(sub,"diagram");

			/* pridej subelement do listWidgetu (actionSelector/taskSelector) */
			listWidget->addItem(subName);
			qDebug() << subName << "registered";
		}	
		else
			qWarning() << "$VIZ_mainWindow::registerSubelement :" << subName << "is not unique name.";
	}	
}

void VIZ_mainWindow::reloadClassTree()
{
	/* definitionEdit je reinicializovan - je treba nastavit novy model */
	if ( classTreeModel != NULL)
		delete classTreeModel;

	/* inicializace grafove struktury */
	QDomElement diagramElement = definitionDocument.firstChildElement("diagram");

	Q_ASSERT(!diagramElement.isNull());

	qDebug() << "$VIZ_mainWindow::reloadClassTree : nacitam diagram";	
	GraphClass graphData(diagramElement);
	graphData.init(NST_CLASS,DEP_INHERITANCE,IGRAPH_DIRECTED);
	
	classTreeModel = new TreeModel(&graphData,this);
	ui.classTreeView->setModel(classTreeModel);
}

void VIZ_mainWindow::updatePropertyEditor()
{
	InfoPanel * infoWidget = static_cast<InfoPanel*>(ui.propertyEditorDock->widget());
       	infoWidget->updateContent();	
}

void VIZ_mainWindow::resetPropertyEditor()
{
	qDebug() << "$VIZ_mainWindow::resetPropertyEditor";
	InfoPanel * infoWidget = new InfoPanel(ui.propertyEditorDock);

	QWidget * oldWidget = ui.propertyEditorDock->widget();
	ui.propertyEditorDock->setWidget(infoWidget);
	infoWidget->show();

	if ( oldWidget != NULL )
		delete oldWidget;
	else
		qWarning() << "$VIZ_mainWindow::resetPropertyEditor : oldWidget == NULL";
}

void VIZ_mainWindow::centerContents(QGraphicsView * view)
{
	QRectF	boundingRect = view->scene()->itemsBoundingRect();
	view->centerOn(boundingRect.center());
}
