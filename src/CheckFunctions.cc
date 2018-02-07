#include "CheckFunctions.h"
#include "XMLHelpFunctions.h"
#include "DataWidget.h"
#include "GraphClass.h"
#include "NodeStructure.h"
#include "EdgeStructure.h"	/* definice DEP_INHERITANCE */
#include <QString>

using namespace XMLh;

QHash<int,QString> CheckFn::predicateArguments(const QDomElement & predicate)
{
	Q_ASSERT(verifyNodeType(predicate,NST_PREDICATE));
	/* <cislo argumentu, label odpovidajiciho uzlu> */
	QHash<int,QString> result;

	QDomElement connections = predicate.firstChildElement("connections");

	if (connections.isNull())
		return result;

	QDomElement diagram = predicate.parentNode().toElement();

	QDomElement actConn = connections.firstChildElement("starts");

	while( !actConn.isNull() )
	{
		/* zjisti cislo argumentu */
		int argNum = getIntAttribute(actConn,"argn");

		/* zjisti ID odpovidajici hrany */
		int edgeID = getIntValue(actConn);
		QDomElement edge = findSubelementAttr(diagram,"edge","id",QString().setNum(edgeID));
	
		Q_ASSERT(!edge.isNull());	
		/* zjisti ID uzlu do ktereho vede */
		int nodeID = associatedNodeID(edge,false);

		QDomElement node = findSubelementAttr(diagram,"node","id",QString().setNum(nodeID));
 
		QString nodeLabel;
		
		/* zjisti label/class ciloveho uzlu */
		if (node.firstChildElement("class").isNull())
			nodeLabel = subelementTagValue(node,"label");	/* argumenty predikatu v definici */
		else
			nodeLabel = subelementTagValue(node,"class");	/* argumenty instance predikatu */
	
		/* cislo argumentu + label/class cile */	
		result.insert(argNum,nodeLabel);

		actConn = actConn.nextSiblingElement("starts");
	}	
	
	return result;
}

bool CheckFn::checkPredicateDefinition(const QDomElement & predicate, bool verbose)
{
	/* hlavni vyznam je v kontrole pretizenych predikatu */
	Q_ASSERT(!predicate.isNull());
	Q_ASSERT(verifyNodeType(predicate,NST_PREDICATE));	

	QString predLabel = subelementTagValue(predicate,"label");
	
	Q_ASSERT(!predLabel.isEmpty());

	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeType,NST_PREDICATE);
	tempNode.setData(NodeStructure::nodeLabel,predLabel);

	QDomElement predParent = predicate.parentNode().toElement();

	/* hledani predikatu se stejnym jmenem */
	QList<int> duplicities = selectMatchingIDList(predParent,tempNode);

	if (verbose)
	{
		/* pokud tam je jeste nejaky takovy */
		if ( duplicities.size() > 1 )
			qDebug() << predLabel << " (overloaded)";
		else
			qDebug() << predLabel;
	}

	/* vypis seznam parametru a zaroven zkontroluj pocty parametru pretizenych predikatu */
	bool equalArgCnt = true;
	int argCnt = -1;	/* inicialni hodnota */	

	foreach(int id,duplicities)
	{
		QDomElement predNode = findSubelementAttr(predParent,"node","id",QString().setNum(id));

		if ( verbose )
		{
			QHash<int,QString> arguments = predicateArguments(predNode);
			
			/* inicializace pri prvnim behu */
			if (argCnt < 0)
				argCnt = arguments.count();

			/* zaznamenani zmeny v poctu argumentu - zmena pouze z true na false */
			if ( equalArgCnt )
				equalArgCnt = (argCnt == arguments.count());

			/* je treba vypsat seznam argumentu pro kazdou verzi */
			if (arguments.isEmpty())
				qDebug() << "No arguments defined";
			else
			{
				for (int i=1; i <= arguments.size(); ++i)
					qDebug() << i << ":" << arguments.value(i);
			}
			qDebug() << "----------------";
		}
		else
		{		
			/* inicializace pri prvnim behu */
			if ( argCnt < 0 )
				argCnt = getArgCount(predNode);
			
			/* zaznamenani zmeny v poctu argumentu - zmena pouze z true na false */
			if ( equalArgCnt )
				equalArgCnt = (argCnt == getArgCount(predNode));
			else
				break;	/* neni treba pokracovat - existuji dve verze s odlisnym poctem argumentu */
		}
	}
	
	if ( !equalArgCnt )
	{
		if (verbose)
			qWarning() << "All versions of overloaded predicate should have equal number of arguments.";

		return false;
	}

	return true;
}

bool CheckFn::checkPredicateInstance(const QDomElement & predicate, const QDomElement & definitionRoot, bool verbose)
{
	Q_ASSERT(!predicate.isNull());
	Q_ASSERT(!definitionRoot.isNull());
	Q_ASSERT(verifyNodeType(predicate,NST_PREDICATE));	

	QString predLabel = subelementTagValue(predicate,"label");
	
	QDomElement predDefinition = selectPredicateDefinition(definitionRoot,predicate);
	if ( predDefinition.isNull() )
	{
		if (verbose)
			qWarning() << "No suitable definition found for" << predLabel;
		return false;
	}

	int defArgCnt = getArgCount(predDefinition);
	int instArgCnt = getArgCount(predicate);

	if ( instArgCnt > defArgCnt )
	{
		if (verbose)
			qWarning() << predLabel << "has too many arguments";
		return false;
	}
	
	if ( instArgCnt < defArgCnt )
	{
		if (verbose)
			qWarning() << predLabel << "has some arguments missing.";
		return false; 
	}

	QDomElement setDefinition = predicate.firstChildElement("set");
	QDomElement stateDefinition = predicate.firstChildElement("state");
	if ( setDefinition.isNull() && stateDefinition.isNull() )
	{
		if (verbose && stateDefinition.isNull())
			qWarning() << predLabel << "has no set defined.";
		
		/* tohle se nemuze stat - stav je pouze v diagramech pro problem:
		 * init/goal */
		/*if (verbose && setDefinition.isNull())
			qWarning() << predLabel << "has no state defined.";
		 */

		return false;
	}

	if (verbose)
		qDebug() << predLabel << "is OK.";

	return true;
}

bool CheckFn::checkClassInstance(const QDomElement & instance, const QDomElement & definitionRoot, bool verbose)
{
	Q_ASSERT(!instance.isNull());
	Q_ASSERT(!definitionRoot.isNull());
	Q_ASSERT(verifyNodeType(instance,NST_VARIABLE) || verifyNodeType(instance,NST_OBJECT));

	QString instClass = subelementTagValue(instance,"class");
	QString instLabel = subelementTagValue(instance,"label");

	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeLabel,instClass);

	int defID = getMatchingNodeID(definitionRoot,tempNode);

	if ( defID != INVALID_ID )
	{
		if (verbose)
			qDebug() << instLabel << "is OK";

		return true;
	}
	else
	{
		if (verbose)
			qWarning() << instLabel << "has invalid class (" << instClass << ")";
		return false;
	}
}

bool CheckFn::checkDefinition(const QDomElement & diagramRoot, bool verbose)
{
	Q_ASSERT(!diagramRoot.isNull());
	Q_ASSERT(diagramRoot.tagName() == "diagram");

	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeType,NST_CLASS);

	QList<QDomElement> classList = selectMatchingElementList(diagramRoot,tempNode);
	/* vypis trid */
	if (verbose)
	{
		qDebug() << "Defined classes:";
		foreach(QDomElement actClass, classList)
		{
			qDebug() << subelementTagValue(actClass,"label");
		}
	}

	/* vypis predikatu */
	if (verbose)
		qDebug() << "Defined predicates:";

	tempNode.setData(NodeStructure::nodeType,NST_PREDICATE);

	QList<QDomElement> predicateList = selectMatchingElementList(diagramRoot,tempNode);

	int totalPredCnt = predicateList.count();

	int predOK = 0;

	QSet<QString> processed;
	foreach(QDomElement actPred, predicateList)
	{
		QString actName = subelementTagValue(actPred,"label");

		if (!processed.contains(actName))
			processed.insert(actName);
		else
		{
			--totalPredCnt; /* v celkovem souctu nesmi byt duplicitni verze zapocitany */
			continue;	/* pretizeny predikat jiz byl zpracovan */
		}

		/* kontrola predikatu */
		if ( checkPredicateDefinition(actPred,verbose) )
			predOK++;
	}

	int predKO = totalPredCnt - predOK;

	if(verbose)
	{
		qDebug() << "------------------------";
		qDebug() << "#classes:" << classList.count();
		
		if ( predKO > 0)
		{
			qDebug() << "#predicates:" << predOK << "+" << predKO << "(valid + invalid)";
			qWarning() << "There are invalid predicates in definition! - check failed";
		}
		else
			qDebug() << "#predicates:" << predOK;
	}

	if ( predKO > 0)
		return false;
	else
		return true;
}

bool CheckFn::checkDependentDiagram(const QDomElement & diagramRoot, const QDomElement & definitionRoot,char rectType, bool verbose)
{
	Q_ASSERT(!diagramRoot.isNull());
	Q_ASSERT(diagramRoot.tagName() == "diagram");
	Q_ASSERT(!definitionRoot.isNull());
	Q_ASSERT(definitionRoot.tagName() == "diagram");
	Q_ASSERT(rectType == NST_VARIABLE || rectType == NST_OBJECT);

	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeType,rectType);

	if (verbose)
	{
		switch(rectType)
		{
			case NST_VARIABLE:
			qDebug() << "Defined variables:";
			break;
			case NST_OBJECT:
			qDebug() << "Defined objects:";
			break;
		}
	}

	/* vypis promennych/objektu */
	QList<QDomElement> rectList = selectMatchingElementList(diagramRoot,tempNode);

	int rectOK = 0;
	int rectKO = 0;

	foreach(QDomElement actRect, rectList)
	{
		if (checkClassInstance(actRect,definitionRoot,verbose))
			rectOK++;
		else
			rectKO++;
	}

	/* vypis predikatu */
	if (verbose)
		qDebug() << "Defined predicates:";

	tempNode.setData(NodeStructure::nodeType,NST_PREDICATE);

	QList<QDomElement> predicateList = selectMatchingElementList(diagramRoot,tempNode);

	int predOK = 0;
	int predKO = 0;

	foreach(QDomElement actPred, predicateList)
	{
		/* kontrola vsech predikatu */
		if ( checkPredicateInstance(actPred,definitionRoot,verbose) )
			predOK++;
		else
			predKO++;
	}

	if (verbose)
	{
		qDebug() << "------------------";

		if ( rectKO > 0 )
		{
			if(rectType == NST_VARIABLE)
			{
				qDebug() << "#variables:" << rectOK << "+" << rectKO << "(valid + invalid)";
				qWarning() << "There are invalid variables (!)";
			}
			if(rectType == NST_OBJECT)
			{
				qDebug() << "#objects:" << rectOK << "+" << rectKO << "(valid + invalid)";
				qWarning() << "There are invalid objects (!)";
			}

		}
		else
		{
			if(rectType == NST_VARIABLE)
				qDebug() << "#variables:" << rectOK;
			if(rectType == NST_OBJECT)
				qDebug() << "#objects:" << rectOK;
		}

		if ( predKO > 0 )
		{
			qDebug() << "#predicates:" << predOK << "+" << predKO << "(valid + invalid)";
			qWarning() << "There are invalid predicates (!)";
		}
		else
		{
			qDebug() << "#predicates:" << predOK;
		}
	}

	if ( (predKO > 0) || (rectKO > 0) )
	{
		if (verbose)
			qWarning() << "Check failed.";
		return false;
	}
	else
		return true;
}

void CheckFn::brokenDiagrams(	const QDomDocument & domain, const QDomElement & definitionRoot,
				const QString & groupTagName, const QString & itemTagName,
				QStringList & correct, QStringList & incorrect)
{
	Q_ASSERT(!domain.isNull());
	Q_ASSERT(!definitionRoot.isNull());

	QDomElement rootEl = domain.firstChildElement();
	Q_ASSERT(rootEl.tagName() == "domain");

	QDomElement groupEl = rootEl.firstChildElement(groupTagName);
	
	Q_ASSERT(!groupEl.isNull());


	correct.clear();
	incorrect.clear();

	char nodeType = NST_UNKNOWN;
	if ( itemTagName == "action" )
		nodeType = NST_VARIABLE;

	if ( itemTagName == "task" )
		nodeType = NST_OBJECT;

	if (nodeType == NST_UNKNOWN)
	{
		qWarning() << "$CheckFn::brokenDiagrams : wrong input";
		return;
	}

	/* najdi prvni clen skupiny (operators -> <action> / problems -> <task>)*/
	QDomElement item = groupEl.firstChildElement(itemTagName);

	while(!item.isNull())
	{
		QString itemName = item.attribute("name");
		QDomElement itemDiagram = item.firstChildElement("diagram");

		if ( checkDependentDiagram(itemDiagram,definitionRoot,nodeType,false) )
			correct << itemName;
		else
			incorrect << itemName; 
	
		item = item.nextSiblingElement(itemTagName);
	}
}

QDomElement CheckFn::selectPredicateDefinition(const QDomElement & definitionRoot, const QDomElement & predInstance)
{
	qDebug() << "$CheckFn::selectPredicateDefinition : selecting best matching definition";

	/* vyber seznam kandidatu na definici */
	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeLabel, subelementTagValue(predInstance,"label"));
	tempNode.setData(NodeStructure::nodeType,NST_PREDICATE);	

	/* vyber vsechny predikaty se stejnym nazvem */
	QList<QDomElement> definitions = selectMatchingElementList(definitionRoot,tempNode);	
	
	/* pokud neni nalezena zadna odpovidajici definice vraci prazdny element */
	if ( definitions.isEmpty() )
		return QDomElement(); 

	QHash<int,QString> instHash = predicateArguments(predInstance);

	/* nejsou instanciovany zadne argumenty - vrat prvni definici */
	if (instHash.isEmpty())
	{
		return definitions.first();
	}

	 /* inicializace grafu urcujiciho dedicnou strukturu */
	GraphClass * inheritanceStructure = new GraphClass(definitionRoot);
	inheritanceStructure->init(NST_CLASS,DEP_INHERITANCE,IGRAPH_DIRECTED);

	QHashIterator<int,QString> iter(instHash);
	while( !definitions.isEmpty() )
	{
		QDomElement currentDef = definitions.takeFirst();

		QHash<int,QString> defHash = predicateArguments(currentDef);

		/* predpokladame, ze aktualni definice je kompatibilni a snazime se to vyvratit */
		bool compatible = true;

		/* resetovani iteratoru */
		iter.toFront();

		/* kontrola vsech instanciovanych argumentu */
		while (iter.hasNext())
		{
			iter.next();
			
			NodeStructure tempNode;
			tempNode.setData(NodeStructure::nodeLabel,iter.value());
			tempNode.setData(NodeStructure::nodeType,NST_CLASS);

			/* zjisti ID tridy tohoto argumentu pro istanci predikatu */
			int instID = getMatchingNodeID(definitionRoot,tempNode);

			/* trida uvedena u prislusneho argumentu v definici */
			QString defClass = defHash.value(iter.key());
			tempNode.setData(NodeStructure::nodeLabel,defClass);

			/* zjisti ID tridy tohoto argumentu pro definici predikatu */
			int defID = getMatchingNodeID(definitionRoot,tempNode);

			if (instID == defID) /* tridy si presne odpovidaji - zkontroluj dalsi argument */
				continue;

			/* ziskej mnozinu s ID nasledniku tridy defClass */
			QSet<int> descSet = inheritanceStructure->getDescendants(defID);

			if ( !descSet.contains(instID) )
			{
				/* tato definice neni kompatibilni - zkus jinou */
				compatible = false;
				break;
			}
 		}     

		if (!compatible)
			continue;

		/* definice je kompatibilni - muzeme skoncit */

		delete inheritanceStructure;

		return currentDef;
	}

	delete inheritanceStructure;

	/* nepodarilo se najit zadnou vhodnou definici */
	return QDomElement();
}

int CheckFn::getArgNumber(const QDomElement & predicate, int edgeID)
{
	Q_ASSERT(!predicate.isNull());

	QDomElement connections = predicate.firstChildElement("connections");

	Q_ASSERT(!connections.isNull());

	QDomElement wantedConn = findSubelementVal(connections,"starts",QString().setNum(edgeID));

	Q_ASSERT(!wantedConn.isNull());

	return getIntAttribute(wantedConn,"argn");	
}

int CheckFn::getArgCount(const QDomElement & predicate)
{
	Q_ASSERT(!predicate.isNull());
	Q_ASSERT(verifyNodeType(predicate,NST_PREDICATE));

	QDomElement connections = predicate.firstChildElement("connections");

	if (connections.isNull())
		return 0;

	return connections.elementsByTagName("starts").count();
}

QStringList CheckFn::classFamily(const QString & className, const GraphClass * structure, const DataWidget * definition)
{
	NodeStructure tempNode;
	tempNode.setData(NodeStructure::nodeLabel,className);
	tempNode.setData(NodeStructure::nodeType,NST_CLASS);
	
	/* zjisti unikatni ID pro tridu jmenem className (toto jmeno je unikatni v ramci domeny) */
	int classID = definition->getMatchingNodeID(tempNode);

	/* pathToRoot obsahuje jmeno tridy className + jmena vsech predku teto tridy
	 * v poradi jak jsou nalezena ve stromu zachycujicim dedicne vztahy */
	return structure->pathToRoot(classID);
}

bool CheckFn::nodeMatches(const QDomElement & probedNode, const NodeStructure & patternNode)
{
	Q_ASSERT(!probedNode.isNull());

	/* promenna pro testovanou hodnotu 
	 * - overeni se provadi pouze pro specifikovane hodnoty (vzdy kdyz neplati: testVal.isNull()) */
	QVariant testVal;

	/* test na shodu labelu */	
	testVal = patternNode.readData(NodeStructure::nodeLabel);
	if ( !testVal.isNull() )
	{
		QDomElement labelEl = probedNode.firstChildElement("label");
		if ( getStrValue(labelEl) != testVal.toString() )		
			return false;
	}

	/* typ uzlu musi byt specifikovan - dalsi testy na nem zavisi */
	char patternNodeType = NST_UNKNOWN;

	/* over typ */
	testVal = patternNode.readData(NodeStructure::nodeType);
	if ( !testVal.isNull() )	
	{
		QDomElement typeEl = probedNode.firstChildElement("type");
        patternNodeType = detNodeType(typeEl);
        if ( patternNodeType != testVal.toChar().toLatin1() )
			return false;
	}

	/* over stav (init/goal) */
	testVal = patternNode.readData(NodeStructure::nodeState);
	if ( (patternNodeType == NST_PREDICATE) && !testVal.isNull() )
	{
		QDomElement stateEl = probedNode.firstChildElement("state");
		if ( getStrValue(stateEl) != testVal.toString() )
			return false;
	}
	
	/* over tridu */
	testVal = patternNode.readData(NodeStructure::nodeClass);
	if ( (patternNodeType != NST_PREDICATE) && !testVal.isNull() )
	{
		QDomElement classEl = probedNode.firstChildElement("class");
		if ( getStrValue(classEl) != testVal.toString() )
			return false;
	}

	/* pokud zadny z testu neselhal, vrat true */
	return true;
}

int CheckFn::getMatchingNodeID(const QDomElement & root, const NodeStructure & nodeTemplate)
{
	QList<int> candidates = selectMatchingIDList(root,nodeTemplate);

	if ( candidates.isEmpty() )
	{
		qWarning() << "$CheckFn::getMatchingNodeID : specified node not found.";
		return INVALID_ID;
	}
	
	if ( candidates.size() > 1 )
		qWarning() << "$CheckFn::getMatchingNodeID : specified node is not unique";

	return candidates.first();
}

QList<int> CheckFn::selectMatchingIDList(const QDomElement & root, const NodeStructure & nodeTemplate)
{
	Q_ASSERT(!root.isNull());

	QList<int> result;

	QDomElement actNode = root.firstChildElement("node");

	while( !actNode.isNull() )
	{
		if ( nodeMatches(actNode,nodeTemplate) )
			result.append(getIntAttribute(actNode,"id"));
		
		actNode = actNode.nextSiblingElement("node");
	}
	
	return result;
}

QList<QDomElement> CheckFn::selectMatchingElementList(const QDomElement & root, const NodeStructure & nodeTemplate)
{
	QList<QDomElement> result;

	QDomElement actNode = root.firstChildElement("node");

	while( !actNode.isNull() )
	{
		if ( nodeMatches(actNode,nodeTemplate) )
			result.append(actNode);
		
		actNode = actNode.nextSiblingElement("node");
	}
	
	return result;
}
