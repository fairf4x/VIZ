#include "Convertor.h"
#include "GraphClass.h"
#include "EdgeStructure.h"	/* definice typu hran */
#include "NodeStructure.h"	/* definice typu uzlu */
#include "XMLHelpFunctions.h"
#include "CheckFunctions.h"
#include <igraph.h>		/* definice IGRAPH_(UN)DIRECTED */
#include <QWidget>
#include <QObject>		/* funkce tr */

#define OUTPUT_INDENT 4

using namespace XMLh;
using namespace CheckFn;

Convertor::Convertor(QWidget * owner)
{
	ownerWidget = owner;
	currentDoc = QDomDocument();
}

QDomDocument Convertor::readFromXML(QFile & input)
{
	QDomDocument domainData;

	QString errorMsg;
	int errorLine;
	int errorColumn;
	
	if ( !domainData.setContent(&input,&errorMsg,&errorLine,&errorColumn) )
	{
		QMessageBox::critical (ownerWidget,"VIZ",
					QObject::tr("Error in XML syntax on line: %1 column: %2").arg(errorLine).arg(errorColumn));
		qCritical() << "Error in XML syntax on line:" << errorLine << "column:" << errorColumn;
		return QDomDocument();
	}

	if ( !basicStructureCheck(domainData) )
	{
		qCritical() << "Basic Structure check failed";
		return QDomDocument();
	}

	return domainData;
}

void Convertor::writeToXML(const QDomDocument & doc, QFile & output)
{
	if (!basicStructureCheck(doc))
	{
		qCritical() << "Error while saving document.";
		return;
	}

	QTextStream saveStream(&output);
	currentDoc.save(saveStream,OUTPUT_INDENT);			
	qDebug() << "Domain succesfully saved.";
}

void Convertor::writeDomainToPDDL(const QDomDocument & doc, QFile & output)
{
	if (!basicStructureCheck(doc))
	{
		qCritical() << "Error while exporting document.";
		return;
	}

	outStream.setDevice(&output);	
	
	/* zapis hlavicky */
	QDomElement description = rootElement.firstChildElement("description");

	if (!description.isNull())
	{
		QStringList lines = getStrValue(description).split("\n");
		foreach(QString line, lines)
		{
			outStream << "; " << line << endl;
		}
		outStream << endl;
	}

	outStream << "(define (domain " << domainName << " )"<< endl;
	
	writeLine(1,"(:requirements :strips :typing)");

	writeItem(1,getDefinedTypes(),"(:types",")");
	writeItem(1,getDefinedPredicates(),"(:predicates",")");

	/* zapis akci */
	QDomElement action = operatorsElement.firstChildElement("action");
	while (!action.isNull())
	{
		QString actionName = action.attribute("name");
		if (actionName.isEmpty())
			qWarning() << "$Convertor::writeDomainToPDDL : missing action name";

		QDomElement actionDiagram = action.firstChildElement("diagram");

		writeAction(1,actionDiagram,actionName);

		action = action.nextSiblingElement("action");
	}

	outStream << ")\n";
	
}

void Convertor::writeProblemsToPDDL(const QDomDocument & doc, QString & targetDir)
{
	if (!basicStructureCheck(doc))
	{
		qCritical() << "Error while exporting document.";
		return;
	}

	QDomElement task = problemsElement.firstChildElement("task");

	while (!task.isNull())
	{
		QString taskName = task.attribute("name");

		if (taskName.isEmpty())
			qWarning() << "$Convertor::writeDomainToPDDL : missing task name";

		QDomElement taskDiagram = task.firstChildElement("diagram");
		writeTask(taskDiagram,taskName,targetDir);

		task = task.nextSiblingElement("task");	
	}
}

bool Convertor::basicStructureCheck(const QDomDocument & domainDocument)
{
	Q_ASSERT(!domainDocument.isNull());
	currentDoc = domainDocument;
	
	rootElement = currentDoc.firstChildElement();
	Q_ASSERT(!rootElement.isNull());

	domainName = rootElement.attribute("name");

	if (domainName.isNull())
		domainName = "UNNAMED";

	definitionElement = rootElement.firstChildElement("definition");
	operatorsElement = rootElement.firstChildElement("operators");
	problemsElement = rootElement.firstChildElement("problems");
	
	if ( definitionElement.isNull() || operatorsElement.isNull() || problemsElement.isNull() )
	{
		qCritical() << "Document syntax error";
		return false;
	}
	else
		return true;
}

void Convertor::writeLine(int indent, QString text)
{
	Q_ASSERT(outStream.device());
	Q_ASSERT(!currentDoc.isNull());
	
	outStream << QString().fill(' ',OUTPUT_INDENT*indent);	
	outStream << text << endl;
}

void Convertor::writeItem(int indent, QString text, QString header, QString footer)
{
	writeLine(indent,header);
	QStringList lines = text.split('\n');

	foreach (QString singleLine,lines)
		writeLine(indent+1,singleLine);

	writeLine(indent,footer);
}

void Convertor::writeAction(int indent, QDomElement diagram, QString name)
{
	qDebug() << "$Convertor::writeAction : current action =" << name;
	writeLine(indent,name.prepend("(:action "));
	
	if (diagram.isNull())
	{
		qWarning() << "Action" << name << "is empty.";
		writeLine(indent,")");
		return;
	}
	
	writeLine(indent+1,getActionParameters(diagram));

	/* mnoziny obsahuji ID predikatu podle toho jestli tam patri */
	QSet<int> actPrecond;
	QSet<int> actEffectPos;
	QSet<int> actEffectNeg;

	/* vsechny predikaty indexovane podle ID */
	QHash<int,QString> allPredicates;	

	/* napln vyse nadefinovane promenne */
	QDomElement actNode = diagram.firstChildElement("node");

	while( !actNode.isNull())
	{
		if ( !verifyNodeType(actNode,NST_PREDICATE) )
		{
			actNode = actNode.nextSiblingElement("node");
			continue;
		}

		int predID = getIntAttribute(actNode,"id");
		/* pridej ID do prislusnych mnozin */
		QDomElement setEl = actNode.firstChildElement("set");
		while(!setEl.isNull())
		{
			QString setStr = getStrValue(setEl);
			
			setEl = setEl.nextSiblingElement("set");

			if ( setStr == NSPS_PRECOND )
			{
				actPrecond.insert(predID);
				continue;
			}

			if ( setStr == NSPS_EFFECT_POS )
			{
				actEffectPos.insert(predID);
				continue;
			}

			if ( setStr == NSPS_EFFECT_NEG )
			{
				actEffectNeg.insert(predID);
				continue;
			}
		}

		/* pridej ID a string do Hash tabulky */	
		QString predStr = getPredicateInstance(actNode,diagram);
		
		allPredicates.insert(predID,predStr);

		actNode = actNode.nextSiblingElement("node");
	}

	/* pomoci mnoziny a Hashe sestav prislusne QStringListy a pouzij je ve funkci buildSetString */
	QStringList positiveList;
	QStringList negativeList;
	
	foreach (int id,actPrecond)
	{
		positiveList << allPredicates.value(id);
	}
	
	/* negativeList je prazdny */
	QString precondString = buildSetString(positiveList,negativeList);
	positiveList.clear();

	foreach (int id,actEffectPos)
		positiveList << allPredicates.value(id);

	foreach (int id,actEffectNeg)
		negativeList << allPredicates.value(id);

	QString effectString = buildSetString(positiveList,negativeList);

	writeItem(indent+1,precondString,":precondition (and ",")");
	writeItem(indent+1,effectString,":effect (and ",")");

	writeLine(indent,")");
}

void Convertor::writeTask(QDomElement & diagram, const QString & name, const QString & targetDir)
{
	Q_ASSERT(!diagram.isNull());

	/* otevrit soubor */
	QStringList fileName;
	fileName << targetDir << "/" << domainName << "_" << name << ".pddl";

	QFile outfile(fileName.join(""));

	if ( !outfile.open(QIODevice::Truncate|QIODevice::WriteOnly) )
	{
		QMessageBox::critical(ownerWidget,QObject::tr("Error"),
				QObject::tr("Can't open file:\n%1").arg(fileName.join("")));
		return;
	}
	
	qDebug() << "$Convertor::writeProblem : writing to file: " << fileName.join("");

	/* nastaveni streamu pro zapis */
	outStream.setDevice(&outfile);	

	outStream << "(define (problem " << name << ")" << endl;
	
	if (!domainName.isEmpty())
	{
		QStringList toWrite;
		toWrite << "(:domain" << domainName << ")";
		writeLine(1,toWrite.join(" "));
	}
	else
		writeLine(1,"(domain UNNAMED)");

	writeItem(1,getObjectList(diagram),"(:objects",")");	

	QStringList initLiterals;
	QStringList goalLiterals;

	QDomElement actNode = diagram.firstChildElement("node");

	while ( !actNode.isNull() )
	{
		if ( !verifyNodeType(actNode,NST_PREDICATE) )	
		{
			actNode = actNode.nextSiblingElement("node");
			continue;
		}

		QString predicateState = subelementTagValue(actNode,"state");

		if ( predicateState == NSTS_INIT )
		{
			initLiterals.append(getPredicateInstance(actNode,diagram,false));
			actNode = actNode.nextSiblingElement("node");
			continue;
		}

		if ( predicateState == NSTS_GOAL )
		{
			goalLiterals.append(getPredicateInstance(actNode,diagram,false));
			actNode = actNode.nextSiblingElement("node");
			continue;
		}

		/* sem by se kod nemel dostat */
		qWarning() << "$Convertor::writeProblem : predicate state is not defined";
	}
	
	writeItem(1,initLiterals.join("\n"),"(:init",")");

	writeLine(1,"(:goal");
	writeItem(2,goalLiterals.join("\n"),"(and ",")");
	writeLine(1,")");

	outStream << ")";

	/* zavrit soubor */	
	outfile.close();
}

QString Convertor::getDefinedTypes()
{
	QDomElement diagramElement = definitionElement.firstChildElement("diagram");
	Q_ASSERT(!diagramElement.isNull());
 
	/* inicializuj GraphClass */
	GraphClass * graph = new GraphClass(diagramElement);

	if (!graph->init(NST_CLASS,DEP_INHERITANCE,IGRAPH_DIRECTED))
	{
		qCritical() << "$Convertor::getDefinedTypes : inheritance graph can was not initialized";
		return QString();
	}

	QString result;
	
	QList<int> queue = graph->childrenNodes(INVALID_ID);	/* koreny jednotlivych komponent souvislosti */

	/* inicializace ID aktualniho rodicovskeho uzlu */
	int currentParent = graph->nodeParentID(queue.first());
	
	qDebug() << "$Convertor::getDefinedTypes : processing queue ...";
	while (queue.size() > 0)
	{
		/*
		pred zacatkem cyklu se stanovi aktualni typ
		- dokud je rodic zpracovavaneho uzlu stejny jako currentParent, pridava se s mezerami
		- pokud se lisi od aktualniho, provede se odradkovani a nastavi se novy currentParent
		- potomci se pridavaji na konec fronty
		*/
		int actNodeID = queue.takeFirst();
		
		if ( currentParent != graph->nodeParentID(actNodeID) )
		{
			/* zakonceni aktualniho radku */
			if (currentParent == INVALID_ID)
				result.append("- object\n");
			else
				result.append(graph->nodeLabel(currentParent).prepend("- ").append("\n"));
			
			/* nastaveni currentParent */
			currentParent = graph->nodeParentID(actNodeID);
		}
		
		/* ulozeni do vysledku */
		result.append(graph->nodeLabel(actNodeID).append(" "));
		
		/* pridani potomku do fronty */
		queue << graph->childrenNodes(actNodeID);

	}
	qDebug() << "$Convertor::getDefinedTypes : queue processed.";
	if (result.isEmpty())
	{
		qWarning() << "There are no types defined in this domain";
		return QString();
	}
	else
	{
		/* zakonceni posledniho radku */
		if ( currentParent != INVALID_ID )
			result.append(graph->nodeLabel(currentParent).prepend("- "));
		else
			result.append("- object");

		return result;
	}
}

QString Convertor::getDefinedPredicates()
{
	QDomElement diagramElement = definitionElement.firstChildElement("diagram");
	Q_ASSERT(!diagramElement.isNull());

	QString result;

	/* vyber vsechny predikaty (podle jmena a pretizene pouze jednou) */
	QSet<QString> predicateSet = selectNodeLabelSet(diagramElement,NST_PREDICATE) ;

	QList<QString> predicateList = predicateSet.toList();
	while (!predicateList.isEmpty())
	{
		result.append( getPredicateDefinition(diagramElement,predicateList.takeFirst()) );
		if (!predicateList.isEmpty())
			result.append("\n");
	}
		
	return result;
}

QString Convertor::getPredicateDefinition(QDomElement diagram, QString predName)
{
	Q_ASSERT(!diagram.isNull());

	/* <poradi argumentu, label argumentu> */
	QMultiMap<int,QString> arguments;

	QDomElement actNode = diagram.firstChildElement("node");
	while (!actNode.isNull())
	{
		/* pokud to neni predikat s danym jmenem pokracuj */
		if (!verifyNodeType(actNode,NST_PREDICATE) || (subelementTagValue(actNode,"label") != predName))
		{
			actNode = actNode.nextSiblingElement("node");
			continue;
		}

		/* ziskani argumentu pro danou definici */
		QHash<int,QString> argHash = predicateArguments(actNode); 
	
		/* pridani do multimapy */
		for (int i=1; i <= argHash.size(); ++i)
		{
			/* stejne typy se nesmi objevit dvakrat */
			if (!arguments.contains(i,argHash.value(i)))
				arguments.insert(i,argHash.value(i));
		}

		actNode = actNode.nextSiblingElement("node");
	} 

	/* seznam typu pro dany predikat */
	QStringList argTypes;

	QList<int> argNumList = arguments.uniqueKeys();
	foreach(int i, argNumList)
	{
		QList<QString> possibleTypes = arguments.values(i);

		if (possibleTypes.size() > 1)
		{
			QStringList typeList(possibleTypes);
			typeList.prepend("(either");
			typeList.append(")");
			argTypes.append(typeList.join(" "));
		}
		else
			argTypes.append(possibleTypes.first());
	}

	/* vygeneruje nazvy argumentu */
	QStringList argNames = generateArgumentNames(argTypes.size(),'a');	

	QString result = buildPredicateString(predName,&argNames,&argTypes);
	
	return result;
}

QString Convertor::getPredicateInstance(QDomElement predicate, QDomElement diagram, bool ques)
{
	Q_ASSERT(!predicate.isNull());

	/* konexe musi byt serazeny podle cisel argumentu - QMap zajistuje trideni */
	QMap<int,int> connections = definedArguments(predicate);		

	QList<int> arguments;

	/* ziskej seznam ID pripojenych uzlu */
	foreach (int id,connections)
	{
		QDomElement edge = findSubelementAttr(diagram,"edge","id",QString().setNum(id));
		arguments.append(associatedNodeID(edge,false));
	}
	
	/* kod funkce se lisi od getPredicateDefinition pouze zpusobem ziskani jmen pro argumenty */
	QStringList argNames = correspondingNodeContent(arguments,diagram,"label");

	qDebug() << "$Convertor::getPredicateInstance : arguments :" << argNames.join(" ");

	/* ziska jmeno zapisovaneho predikatu */
	QString predName = subelementTagValue(predicate,"label");

	QString result = buildPredicateString(predName,&argNames,(QStringList*)0,ques);
	
	return result;
}

QString Convertor::getObjectList(QDomElement diagram)
{
	Q_ASSERT(!diagram.isNull());

	QDomElement actNode = diagram.firstChildElement("node");
	
	QMultiMap<QString,QString> objectMap;
	QStringList keyList;
	
	while (!actNode.isNull())
	{
		if ( !verifyNodeType(actNode,NST_OBJECT) )
		{
			actNode = actNode.nextSiblingElement("node");
			continue;
		}

		QString objectName = subelementTagValue(actNode,"label");
		QString objectClass = subelementTagValue(actNode,"class");

		objectMap.insert(objectClass,objectName);
		if ( !keyList.contains(objectClass) )
			keyList.append(objectClass);

		actNode = actNode.nextSiblingElement("node");	
	}	

	QString result;
	while ( !keyList.isEmpty() )
	{
		QString currentClass = keyList.takeFirst();
		QList<QString> members = objectMap.values(currentClass);

		/* vyjmenuj objekty stejneho typu */
		foreach (QString obj, members)
		{
			result.append(" ");
			result.append(obj);
		}
		
		/* pridej jmeno tridy a odradkuj */
		result.append(" - ");
		result.append(currentClass);
		if (!keyList.isEmpty())
			result.append("\n");
	}
	
	return result;
}

QString Convertor::buildSetString(QStringList & positive, QStringList & negative)
{
	/* pozitivni predikaty staci odradkovat */
	QString result = positive.join("\n");

	/* zabaleni negativnich predikatu "do negace" */
	foreach(QString pred,negative)
	{
		result.append("\n(not ");
		result.append(pred);
		result.append(")");
	}			

	return result;
}

QString Convertor::buildPredicateString(QString & name, QStringList * argNames, QStringList * argTypes, bool ques)
{
	QString result;

	result.append("(");
	result.append(name);
	
	while( !argNames->isEmpty() )
	{
		if (ques)
			result.append(" ?");
		else
			result.append(" ");

		result.append(argNames->takeFirst());

		if (argTypes && !argTypes->isEmpty())
		{
			result.append(" - ");
			result.append(argTypes->takeFirst());
		}
	}	

	result.append(")");

	return result;
}

QStringList Convertor::generateArgumentNames(int size, char startingChar)
{
	QStringList result;


	for (int i = 0; i < size; ++i)
	{
		QChar test(startingChar + i);

		if (!test.isPrint())
			qWarning() << "$Convertor::generateArgumentNames : non printable character";

		result << QChar(startingChar+i);
	}
	
	return result;
}

QString Convertor::getActionParameters(QDomElement diagram)
{
	Q_ASSERT(!diagram.isNull());

	QString result = ":parameters (";
	QDomElement actNode = diagram.firstChildElement("node");

	while( !actNode.isNull() )
	{
		if ( !verifyNodeType(actNode,NST_VARIABLE) )
		{
			actNode = actNode.nextSiblingElement("node");
			continue;
		}

		QString varName = subelementTagValue(actNode,"label");
		QString varType = subelementTagValue(actNode,"class");

		result.append("?");
		result.append(varName);
		result.append(" - ");
		result.append(varType);

		actNode = actNode.nextSiblingElement("node");
		if (!actNode.isNull())
			result.append(" ");
	}
	
	result.append(")");		

	return result;
}
