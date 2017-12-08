#include <QtGlobal>
#include "XMLHelpFunctions.h"
#include "GraphClass.h"
/* kvuli definici typu hran pro edgeMask */
#include "EdgeStructure.h"

using namespace XMLh;

GraphClass::GraphClass(QDomElement diagram)
{
	graphData = diagram;
	igraph_empty(&graph,0,false);
}

GraphClass::~GraphClass()
{
	igraph_destroy(&graph);
}

bool GraphClass::init(char nodeMask,char edgeMask, bool directed)
{
	/* spocitej hrany a inicializuj pro ne vektor */	
	QList<EdgeDefinition> edgeList = selectEdges(graphData,edgeMask); 
	int edgeCnt = edgeList.size();
	if (edgeCnt == 0)
		qDebug() << "$GraphClass::init : no edges in diagram";

	igraph_vector_t edgeVect;
	igraph_vector_init(&edgeVect,edgeCnt*2);	

	/* zjisti pocet uzlu a inicializuj graf */
	QList<int> nodes = selectNodeIDList(graphData,nodeMask);
	if ( nodes.count() == 0 )
	{
		qDebug() << "$GraphClass::init : no nodes in diagram";
		return false;		
	}

	igraph_destroy(&graph);
	igraph_empty(&graph,nodes.count(),directed);
	
	/* namapuj nodeID na vertexID */
	for (int i = 0; i < nodes.count(); ++i)
	{
		/* namapuj aktualni uzel */	
		idMap.insert(nodes.at(i),i);		
	}	

	/* vsechny uzly uz jsou namapovany */

	/* napln vektor hran podle namapovanych hodnot */
	for (int i = 0; i < edgeCnt; ++i)
	{
		VECTOR(edgeVect)[2*i] = idMap.value(edgeList.at(i).first);
		VECTOR(edgeVect)[2*i+1] = idMap.value(edgeList.at(i).second);
	}
	
	igraph_add_edges(&graph,&edgeVect,0);

	igraph_vector_destroy(&edgeVect);
	
	directedHasCycle = false;
	return true;
}

bool GraphClass::containsCycle()
{
	/* pokud je graf orientovany, nastavi se promenna directedHasCycle */
	selectComponentReprezentants();
	
	return directedHasCycle;	
}

int GraphClass::edgeCnt(int nodeID, igraph_neimode_t mode)
{
	igraph_vector_t res;
	igraph_vector_init(&res,1);
	
	igraph_degree(&graph, &res, igraph_vss_1((igraph_integer_t)idMap.value(nodeID)), mode, IGRAPH_NO_LOOPS);

	return (int)VECTOR(res)[0];
}

QString GraphClass::nodeLabel(int nodeID) const
{
	QDomElement node = findSubelementAttr(graphData,"node","id",QString().setNum(nodeID));

	Q_ASSERT(!node.isNull());
	
	return subelementTagValue(node,"label");
}

int GraphClass::nodeParentID(int nodeID) const
{
	igraph_integer_t vertID = idMap.value(nodeID,INVALID_ID);

	Q_ASSERT(vertID != INVALID_ID);

	igraph_vector_ptr_t res;
	igraph_vector_ptr_init(&res,1);

	/* do vektoru res ulozi id vsech uzlu ze kterych je vertID dosazitelny v jednom kroku
	 * + sebe sama na prvni pozici */
	igraph_neighborhood(&graph, &res,igraph_vss_1(vertID), 1,IGRAPH_IN,0);

	/* z vysledku vezmi prvni ukazatel na vektor "sousedu" */
	igraph_vector_t * parentVect = (igraph_vector_t*) igraph_vector_ptr_e(&res,0);

	/* ve vysledku je obsazen i uzel samotny - z toho duvodu 2 */	
	if ( igraph_vector_size(parentVect) > 2)
		qWarning() << "$GraphClass::nodeParentID : uzel ma vic nez 1 rodice";

	if (igraph_vector_size(parentVect) < 2 )
	{
		qDebug() << "$GraphClass::nodeParentID : uzel nema rodice";
		igraph_vector_destroy(parentVect);
		igraph_vector_ptr_destroy(&res);
		return INVALID_ID;
	}

	/* z vektoru precti druhy prvek (na indexu 1) */
	igraph_integer_t parentID = VECTOR(*parentVect)[1];
	igraph_vector_destroy(parentVect);
	igraph_vector_ptr_destroy(&res);
	return idMap.key((int)parentID);
}

void GraphClass::addEdge(int startID,int endID)
{
	/* INVALID_ID je defaultni hodnota v pripade ze by tam uzel nebyl */
	igraph_integer_t startVertex = idMap.value(startID,INVALID_ID);
	igraph_integer_t endVertex = idMap.value(endID,INVALID_ID);

	Q_ASSERT( (startVertex != INVALID_ID) && (endVertex != INVALID_ID) );

	igraph_add_edge(&graph,startVertex,endVertex);		
}

void GraphClass::removeEdge(int startID, int endID)
{
	igraph_integer_t startVertex = idMap.value(startID,-1);
	igraph_integer_t endVertex = idMap.value(endID,-1);
	
	Q_ASSERT( (startVertex != INVALID_ID) && (endVertex != INVALID_ID) );

	igraph_integer_t edgeID;
	
	igraph_get_eid(&graph, &edgeID, startVertex, endVertex, igraph_is_directed(&graph), false);

	igraph_delete_edges(&graph,igraph_ess_1(edgeID));
}

QStringList GraphClass::pathToRoot(int pathStart) const
{
	Q_ASSERT(pathStart != INVALID_ID);

	QStringList result;
	while ( pathStart != INVALID_ID )
	{
		result.append(nodeLabel(pathStart));
		pathStart = nodeParentID(pathStart);
	}	

	return result;
}

QList<int> GraphClass::childrenNodes(int parent)
{
	/* pokud je parametr parent neplatny uzel (<= 0),
	vrati seznam vybranych uzlu - z kazde komponenty souvislosti 1 */
	if (parent <= 0)
	{
		return selectComponentReprezentants();
	}
	else
	{
		return selectNeighbours(parent);
	}
}

QSet<int> GraphClass::getDescendants(int parent)
{
	QSet<int> result;

	/* nejdriv do fronty pridej prime potomky */
	QList<int> queue = childrenNodes(parent);

	while( !queue.isEmpty() )
	{
		/* zpracuj prvni prvek fronty */
		int descID = queue.takeFirst();

		/* pridej do fronty potomky zpracovavaneho naslednika */
		queue << childrenNodes(descID);

		/* pridej aktualniho naslednika do vysledne mnoziny */
		result << descID;
	}

	return result;
}

void GraphClass::print()
{
	qDebug() << "GraphClass::print : node cnt:" << (int)igraph_vcount(&graph) <<
					"edge cnt:" << (int)igraph_ecount(&graph) << endl;
}

QList<int> GraphClass::selectComponentReprezentants()
{
	QList<int> result;
	directedHasCycle = false;

	igraph_integer_t vertexCnt = igraph_vcount(&graph);
	
	/* vektor pro vrcholy jedne komponenty */
	igraph_vector_t list;

	QStack<igraph_integer_t> vertexes;
	igraph_integer_t i;	/* iteracni promenna */

	/* inicializace zasobniku */
	for (i=0; i<vertexCnt; ++i)
		vertexes.push(i);

	/* pokud je graf orientovany, jedna se o les (zajimaji nas koreny stromu - jedna se o strukturu trid) */
	if ( igraph_is_directed(&graph) )
	{
		qDebug() << "$GraphClass::selectComponentReprezentants : hledam koreny lesa";
		/* vektor obsahujici pocet vstupnich hran pro kazdy vrchol grafu */
		igraph_vector_t degrees;
		igraph_vector_init(&degrees,vertexCnt);
		igraph_degree(&graph,&degrees,igraph_vss_all(),IGRAPH_IN,IGRAPH_NO_LOOPS);

		/* index uzlu, ktery je korenem zkoumane komponenty */
		igraph_integer_t rootVertex;
		/* minimalni pocet vstupnich hran ve zkoumane komponente */
		igraph_integer_t minimum;

		while (!vertexes.isEmpty())
		{
			/* vezmi vrchol ze zasobniku a urci jeho pocet vstupnich hran */
			rootVertex = vertexes.pop();
			minimum = igraph_vector_e(&degrees,rootVertex);
		
			/* inicializace seznamu vertexu z komponenty */	
			igraph_vector_init(&list,0);
			igraph_subcomponent(&graph,&list,rootVertex,IGRAPH_ALL);
		
			igraph_integer_t listLength = igraph_vector_size(&list);

			for (i=0; i<listLength; ++i)
			{	
				/* zjisteni ID zkoumaneho uzlu */
				igraph_integer_t actID = igraph_vector_e(&list,i);
				
				/* preskoc uzel shodny s prvnim vybranym - uz neni v zasobniku */
				if (actID == rootVertex)
					continue;

				/* zjisteni poctu jeho vstupnich hran */
				igraph_integer_t inputDeg = igraph_vector_e(&degrees,actID);
			
				/* odebrani ze zasobniku */
				vertexes.remove(vertexes.indexOf(actID));
						
				/* update indexu s nejmensim nalezenym poctem hran */
				if (inputDeg < minimum)
				{
					minimum = inputDeg;
					rootVertex = actID;
				}
			}
			
			if (minimum == 0)
			{
				qDebug() << "$Nasel jsem koren!";
				result.append(idMap.key(rootVertex));
			}
			else
			{
				qWarning() << "$GraphClass::selectComponentReprezentants : graph contains a cycle";
				directedHasCycle = true;
			}

			igraph_vector_destroy(&list);
		}

		igraph_vector_destroy(&degrees);
	}
	else
	/* pokud neni orientovany, vrat seznam nahodne vybranych uzlu */
	{

		qDebug() << "$GraphClass::selectComponentReprezentants : vybiram nahodne reprezentanty";
		while (!vertexes.isEmpty())
		{
			/* nejaky vrchol */
			igraph_integer_t someVertex = vertexes.pop();
			result.append(idMap.key(someVertex));
			
			/* inicializace seznamu vertexu z komponenty */	
			igraph_vector_init(&list,0);
			igraph_subcomponent(&graph,&list,someVertex,IGRAPH_ALL);

			/* odebrani vsech vertexu teto komponenty ze zasobniku (krome someVertex - ten uz tam neni) */
			for (i=0; i<igraph_vector_size(&list); ++i)
			{
				igraph_integer_t actID = igraph_vector_e(&list,i); 
				if (actID == someVertex)
					continue;
				vertexes.remove(vertexes.indexOf(actID));
			}

			igraph_vector_destroy(&list);
		}
		
	}
	
	return result;
}

QList<int> GraphClass::selectNeighbours(int parent)
{
	QList<int> result;
	igraph_integer_t i; /* iteracni promenna */
	
	/* preved ID rodice */
	igraph_integer_t parentVertex = idMap.value(parent);

	igraph_vector_t neighbours;
	igraph_vector_init(&neighbours,0);
	
	/* zjisti seznam sousedu
	 * (pokud graf neni orientovany funkce igraph_neighbors posledni paramatr ignoruje) */
	igraph_neighbors(&graph,&neighbours,parentVertex,IGRAPH_OUT);

	/* pridej vsechny sousedy do vysledneho seznamu (s prevedenymi id) */
	for (i=0; i<igraph_vector_size(&neighbours); ++i)
	{
		igraph_integer_t actID = igraph_vector_e(&neighbours,i);
		result.append(idMap.key(actID));
	}
	
	/* znic sousedy */
	igraph_vector_destroy(&neighbours);	

	return result;
}
