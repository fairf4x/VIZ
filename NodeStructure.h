#ifndef NODESTRUCTURE_H
#define NODESTRUCTURE_H

#include <QtCore>
#include "Node.h"

#define MAX_LABEL_LENGTH 20

/* node colors */
#define NC_CLASS "#F8F06F"
#define NC_VARIABLE "#B1FB3C"
#define NC_OBJECT "#4ADADC"
#define NC_PREDICATE "#F7AE22"
#define NC_INIT_PREDICATE "#75F4AB"
#define NC_GOAL_PREDICATE "#FA8C80"
#define NC_DEFAULT "#FFFFFF"

/* predicate layer colors */
#define LC_PRECOND "#3CDC84"
#define LC_EFFECT_POS "#0F74DC"
#define LC_EFFECT_NEG "#FF5753"
#define LC_DEFAULT "#FFFFFF"

/* definice bitovych map pro typ uzlu a prislusnost do mnoziny */
#define NS_DEFAULT_ID		-1
#define NS_UNDEF		0	/*0000 0000*/	

#define NST_CLASS		(char)1		/*0000 0001*/
#define NST_PREDICATE		(char)2		/*0000 0010*/
#define NST_VARIABLE		(char)4		/*0000 0100*/
#define NST_OBJECT		(char)8		/*0000 1000*/
#define NST_ALL_TYPES		(char)15	/*0000 1111*/	
#define NST_UNKNOWN		(char)240	/*1111 0000*/

#define NSPS_PRECOND		"precond"
#define NSPS_EFFECT_POS		"effect+"
#define NSPS_EFFECT_NEG		"effect-"

#define NSTS_INIT		"init"
#define NSTS_GOAL		"goal"

struct NodeStructure
{
	NodeStructure();
	NodeStructure(const NodeStructure & original);

	enum ContentType {	nodeID, 
				nodeType, 
				nodeLabel, 
				nodePosition, 
				nodePredicateSet,	/* specificke pro predikaty */
				nodeState,		/* specificke pro predikaty v TaskEdit */
				nodeClass		/* specificke pro promenne a objekty */
			 };

	QVariant readData(ContentType type) const;
	void setData(ContentType type, QVariant data);

	Node::NodeShape	shape() const;
	QColor		color() const;
	private:
		QHash<ContentType,QVariant> content; 
};

#endif
