/** @file editorobjmodel.cpp
* @brief the C++ object model.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the C++ object model Class.
*/

#include "editorobjmodel.h"
#include "cppfunction.h"
#include "cppkeywords.h"
#include "editor1.h"
#include "assert.h"
#include <QStandardItemModel>
#include <QSet>

CppObjectModel::CppObjectModel(QObject *parent) :
    QAbstractItemModel (parent),perscount(0)
{}

QModelIndex CppObjectModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
    if (column != 0)
        return QModelIndex();

    QStandardItem *childItem = 0;
    if(!parent.isValid())
    {
        int _row = row;
        foreach (QStandardItemModel *it, models)
        {
            if(_row < it->rowCount() )
            {
                childItem = it->item(_row);
                break;
            }
            _row -= it->rowCount();
        }
        if(!childItem)
            childItem = Editor1::source().include;
    }
    else
        childItem = itemFromIndex(parent)->child(row);

    if (childItem)
    {
        childItem->setData(parent,Item::ParentIndexRole);
        return createIndex(row, column, childItem);
    }
    else
        return QModelIndex();
}

QModelIndex CppObjectModel::parent( const QModelIndex &child ) const
{
    if (!child.isValid() )
        return QModelIndex();
    assert(itemFromIndex(child));
    return itemFromIndex(child)->data(Item::ParentIndexRole).toModelIndex();
}

int CppObjectModel::rowOf(QStandardItemModel *model) const
{
    int midx = models.indexOf(model);
    int count = 0;
    for(int i=0;i<midx;++i)
        count += models[i]->rowCount();
    return count;
}

int CppObjectModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        int count = 1;
        foreach (QStandardItemModel *it, models)
            count += it->rowCount();
        return count;
    }

    return itemFromIndex(parent)->rowCount();
}

int CppObjectModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant CppObjectModel::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role) const
{
    if(orientation == Qt::Vertical)
        return QAbstractItemModel::headerData(section,orientation,role);

    switch(role)
    {
    case Qt::DisplayRole:
        return "Variables";
    default:
        return QAbstractItemModel::headerData(section,orientation,role);
    }
}

QVariant CppObjectModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();
    return itemFromIndex(index)->data(role);
}

QStandardItem * CppObjectModel::itemOf(const QString &name,const QString &scope) const
{
    QStandardItem *item = itemOf(scope+name);
    return item;
}

//---- find item of path------
QStandardItem * CppObjectModel::itemOf(const QString &path) const
{
    QStringList list = path.split("::");

    QModelIndex index;
    foreach (const QString &it,list)
    {
        assert( !it.isEmpty());
        if( ! ( index = findChild(it,index) ).isValid() )
            return 0;
    }

    return itemFromIndex(index);
}

QModelIndex CppObjectModel::findChild(const QString &name, const QModelIndex &parent) const
{
    for(int row=0; row<rowCount(parent); ++row)
    {
        QModelIndex idx = index(row,0,parent);
        if(idx.data().toString() == name)
            return idx;
    }
    return QModelIndex();
}

void CppObjectModel::appendPersistentModel(QStandardItemModel *model)
{
    assert(model);

    if(models.contains(model))
        return;

    if(! model->rowCount())
    {
        models.append(model);
        ++perscount;
        return;
    }

    const int first = rowCount()-1; // the last item is include item;
    const int last = first + model->rowCount()-1;
    beginInsertRows(QModelIndex(),first,last);
    models.append(model);
    endInsertRows();
    ++perscount;
}

void CppObjectModel::prependModel(QStandardItemModel *model)
{
    assert(model);

    if(models.contains(model))
        return;

    if(! model->rowCount())
    {
        models.prepend(model);
        return;
    }

    beginInsertRows(QModelIndex(),0,model->rowCount()-1);
    models.prepend(model);
    endInsertRows();
}

void CppObjectModel::clear()
{
    const int count = models.size()-perscount;
    //keep all
    if(count <= 0)
        return;

    int last = -1;
    for (int i=0; i < count; ++i)
        last += models[i]->rowCount();

    if(last == -1)
    {
        models.remove(0,count);
        return;
    }

    beginRemoveRows(QModelIndex(),0,last);
    models.remove(0,count);
    endRemoveRows();
}
