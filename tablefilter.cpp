/** @file tablefilter.cpp
* @brief the data filter of the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the data filter of the data table.
*/

#include "tablefilter.h"
#include "table.h"
#include "mdparout.h"
#include "item.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QModelIndex>
#include <QToolTip>
#include <QDir>

TableFilter::TableFilter(QObject *parent) : QSortFilterProxyModel(parent) { }
TableResult *TableFilter::sourceModel() const
{ return static_cast<TableResult *>(QSortFilterProxyModel::sourceModel()); }

void TableFilter::setSourceModel(TableResult *sourceModel)
{ QSortFilterProxyModel::setSourceModel(sourceModel); }

void TableFilter::invalidateFilter()
{ QSortFilterProxyModel::invalidateFilter(); }

int TableFilter::parameterColumnCount() const
{
    assert(sourceModel());
    return sourceModel()->parameterColumnCount();
}

bool TableFilter::needHide(int section) const
{
    assert(sourceModel() && sourceModel()->pm);
    assert(section > 0 && section < sourceModel()->parameterColumnCount());

    if(! sourceModel()->pm->item(section-1) )
    {
        QToolTip::showText(QCursor::pos(),tr("The Table and The Model is not consistent!"));
        return false;
    }

    return sourceModel()->pm->item(section-1)->data(Item::HeaderFlagRole).toInt() == Item::Parafiter;
}

QString TableFilter::headerDataForCopy(int column) const
{
    assert(sourceModel());
    return sourceModel()->headerDataForCopy(column);
}

QString TableFilter::dataForCopy(const QModelIndex &idx) const
{
    assert(sourceModel());
    return sourceModel()->dataForCopy(mapToSource(idx));
}

bool TableFilter::isFile(const QModelIndex &idx) const
{
    assert(sourceModel());
    return sourceModel()->isFile(mapToSource(idx));
}

QString TableFilter::directoryPath(const QModelIndex &idx) const
{
    assert(sourceModel());
    return sourceModel()->directoryPath(mapToSource(idx));
}

bool TableFilter::removeFile(const QString &key, const QString &fileName) const
{
    bool ok = sourceModel()->pm->removeFile(key,fileName);
    sourceModel()->calculateAll();
    return ok;
}

int TableFilter::generateMain(const QString &mainsource, const QString &path, QStringList &idList, QWidget *parent) const
{
    int numseeds = 0;

    QFile rf(mainsource);
    if(!rf.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(parent,tr("Error"),tr("Failed to open the file:\n") + mainsource,QMessageBox::Ok,QMessageBox::Ok);
        return 0;
    }
    const QString omain = rf.readAll();

    QDir dir(path);
    idList.clear();
    for (int row=0; row< rowCount()-1; ++row)
    {
        QString key = headerData(row,Qt::Vertical).toString();
        int times = index(row,sourceModel()->parameterColumnCount()).data().toInt();

        if(times < 1 || !dir.mkdir(key) || !dir.cd(key))
            continue;

        numseeds += times;
        idList << key;

        QFile fm(dir.absoluteFilePath("main.cpp"));
        if(!fm.open(QIODevice::WriteOnly|QIODevice::Truncate) )
        {
            QMessageBox::critical(parent,tr("Error"),tr("Failed to create the file:\n") + fm.fileName(),QMessageBox::Ok,QMessageBox::Ok);
            return 0;
        }
        QString cmain = cmainOf(row,omain);
        fm.write(cmain.toLocal8Bit());

        QFile ft(dir.absoluteFilePath("times"));
        if(!ft.open(QIODevice::WriteOnly|QIODevice::Truncate) )
        {
            QMessageBox::critical(parent,tr("Error"),tr("Failed to create the file:\n") + ft.fileName(),QMessageBox::Ok,QMessageBox::Ok);
            return 0;
        }
        ft.write(QString::number(times).toLocal8Bit());

        dir.cdUp();
    }

    return numseeds;
}

QString TableFilter::cmainOf(int row, QString omain) const
{
    for (int j = 1;j<sourceModel()->columnCount();++j)
    {
        const QString para = headerData(j,Qt::Horizontal).toString();
        QRegExp rx("\\b"+para+"\\s*=\\s*\\?\\s*;");
        omain.replace(rx,para+" = "+index(row,j).data().toString()+";");
    }
    return omain;
}

bool TableFilter::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    assert(sourceModel());
    QStandardItem *vItem = sourceModel()->verticalHeaderItem( source_row);
    if( !vItem || vItem->data(Item::DataRole).toInt() == Item::OtherRow)
        return true;

    for (int j = 0; j < sourceModel()->parameterColumnCount(); ++j)
    {
        QStandardItem *hItem = sourceModel()->horizontalHeaderItem( j);
        if( hItem && hItem->data(Item::HeaderFlagRole).toInt() == Item::Parafiter &&
               sourceModel()->index(source_row,j).data().toDouble() != hItem->data(Item::DataRole).toDouble()  )
            return false;
    }
    return true;
}

bool TableFilter::isInOtherRow(const QModelIndex &index) const
{    
    return sourceModel()->headerData(index.row(),Qt::Vertical,Item::DataRole).toInt() == Item::OtherRow;
}

bool TableFilter::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if( isInOtherRow(right))
        return sortOrder() == Qt::AscendingOrder;
    else if( isInOtherRow(left))
        return sortOrder() == Qt::DescendingOrder;
    else
        return left.data().toDouble() < right.data().toDouble();
}

void TableFilter::addFilter(const QModelIndex &index)
{
    QStandardItem *vItem = sourceModel()->verticalHeaderItem(index.row());
    if( ! index.data().isValid() || ! vItem || vItem->data(Item::DataRole).toInt() == Item::OtherRow )
        return;
    addFilter(index.column(),index.data());
}

void TableFilter::deleteFilter(const QModelIndex &index)
{
    QStandardItem *vItem = sourceModel()->verticalHeaderItem(index.row());
    if(! vItem )
        return;
    deleteFilter(index.column());
}

void TableFilter::addFilter(int section, const QVariant &val)
{
    QStandardItem *hItem = sourceModel()->horizontalHeaderItem(section);
    if( ! val.isValid() || (! hItem || ! (hItem->data(Item::HeaderFlagRole).toInt() & (Item::Parameter | Item::Parafiter) )) )
        return;

    hItem->setData(val,Item::DataRole);
    hItem->setData(Item::Parafiter,Item::HeaderFlagRole);
    QFont font = hItem->font();
    font.setUnderline(true);
    hItem->setFont(font);
    invalidate();
}

void TableFilter::deleteFilter(int section)
{
    QStandardItem *hItem = sourceModel()->horizontalHeaderItem(section);
    if( ! hItem || hItem->data(Item::HeaderFlagRole).toInt() != Item::Parafiter )
        return;

    hItem->setData(Item::Parameter,Item::HeaderFlagRole);
    QFont font = hItem->font();
    font.setUnderline(false);
    hItem->setFont(font);

    invalidate();
}
