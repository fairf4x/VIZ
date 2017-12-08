#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QMap>

#define LABEL_COLUMN 0
#define ID_COLUMN 1

class TreeItem;
class GraphClass;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(GraphClass *graph, QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());

    bool removeRow(int id);

private:
    void initClasses(TreeItem *parent, int parentID);
    TreeItem *getItem(const QModelIndex &index) const;
    QModelIndex getItemIndex(int id);
    QModelIndex recursiveSearch(const QModelIndex & parent, QString label);
    
    TreeItem *rootItem;
    GraphClass * graphData;

    QString lastInserted;
    QMap<int,QString> modelMap;
};

#endif
