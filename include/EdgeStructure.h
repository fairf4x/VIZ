#ifndef EDGESTRUCTURE_H
#define EDGESTRUCTURE_H

#include <QtCore>
#include "Edge.h"

/* definice moznych vyznamu hran */
#define DEP_INHERITANCE 2	/* 0000 0010 */
#define DEP_ASSOCIATION 1	/* 0000 0001 */
#define DEP_NO_PURPOSE	0	/* 0000 0000 */
#define DEP_UNKNOWN	252	/* 1111 1100 */

/* edge colors */
#define EC_ASSOCIATION "#5E5F76"
#define EC_INHERITANCE "#1386CF"


struct EdgeStructure
{
	int id;
	char purpose;

	QPointF startPos;
	QPointF endPos;

	int startNodeID;
	int endNodeID;

	Edge::EdgeOrientation orientation() const;
	QPen  pen() const;
	
	void switchOrientation();
};

#endif
