#include <QtGui>

#include "treeitem.h"
#include "treemodel.h"
#include "DataWidget.h"
#include "GraphClass.h"
#include "CustomEvents.h"

#define COLUMN_CNT 1

#include <iostream>
using namespace std;

TreeModel::TreeModel(GraphClass *graph, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    rootData << tr("Class name");

    graphData = graph;

    rootItem = new TreeItem(rootData);
    /* rootItem ma neplatne ID*/
    initClasses(rootItem,0); 

    lastInserted = QString();
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
	qWarning() << "$TreeModel::data : invalid index";
        return QVariant();
    }

    if (role == Qt::UserRole )
    {
	/* najdi label */
    	TreeItem *item = getItem(index);
	QString label = item->data(index.column()).toString();

	/* vrat odpovidajici ID, ktere je uvedene v modelMap */
	return QVariant(modelMap.key(label));
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole )
        return QVariant();

    TreeItem *item = getItem(index);

    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}

QModelIndex TreeModel::getItemIndex(int id)
{
	QString label = modelMap.value(id); 	
	QModelIndex parentIndex = QModelIndex();
	return recursiveSearch(parentIndex,label);
}

QModelIndex TreeModel::recursiveSearch(const QModelIndex & parent, QString label)
{
	/* konec rekurze */
	if ( data(parent,Qt::EditRole).toString() == label)
		return parent;

	/* projdi vsechny potomky */
	TreeItem * actItem = getItem(parent);
	QModelIndex child;
	QModelIndex result = QModelIndex();
	for (int row = 0; row < actItem->childCount(); ++row)
	{
		child = index(row,LABEL_COLUMN,parent);
		
		result = recursiveSearch(child,label);
		
		QString resLabel = data(child,Qt::EditRole).toString();

		if( resLabel == label)
			break;
	}

	return result;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, COLUMN_CNT);
    endInsertRows();

    return success;
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

bool TreeModel::removeRow(int id)
{
	QModelIndex removedIndex = getItemIndex(id);
	
	/* tridu nelze odebrat pokud ma potomky */
	TreeItem * removedItem = getItem(removedIndex);
	if ( removedItem->childCount() > 0 )
		return false;

	return removeRows(removedIndex.row(),1,removedIndex.parent());	
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 1;
}
int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value,
                        int role)
{
    TreeItem * item;
    switch(role)
    {
	case Qt::EditRole:
		item = getItem(index);
		lastInserted = value.toString();
		return item->setData(index.column(), value);
	break;
	case Qt::UserRole:
		/* pridej zaznam do mapy - pokud byl jiz vlozen prislusny label
		 * nejdriv je treba vlozit label a potom teprve ID ktere k nemu patri */
		Q_ASSERT( !lastInserted.isEmpty() );
		
		modelMap.insert(value.toInt(),lastInserted);
		lastInserted = QString();
		return true;
	break;
	default:
		return false;
    }
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    return rootItem->setData(section, value);
}

void TreeModel::initClasses(TreeItem * parent, int parentID)
{
	QList<int> children = graphData->childrenNodes(parentID);

	int childCnt = children.size();

	/* ukonceni rekurze */
	if ( childCnt <= 0)
		return;
	
	/* pridani vsech potomku */

	/* pridani childCnt novych radku (pridavame od 0teho radku) */
	parent->insertChildren(0,childCnt,COLUMN_CNT);

	/* naplneni novych radku daty */
	for (int i=0; i<childCnt; ++i)
	{
		int childID = children.at(i);
		QString childLabel = graphData->nodeLabel(childID);
		/* sem se jeste nejak musi propasovat informace o nazvu tridy */
		parent->child(i)->setData(LABEL_COLUMN,childLabel);
		
		/* vlozeni zaznamu do mapy */
		modelMap.insert(childID,childLabel);
	
		/* rekurzivni volani */
		initClasses(parent->child(i),childID);
	}
}
