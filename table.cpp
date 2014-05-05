/** @file table.cpp
* @brief the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the data table.
*/

#include "table.h"
#include "tablecalculator.h"
#include "mdparout.h"
#include "item.h"
#include "assert.h"
#include <QDir>
#include <QUrl>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QToolTip>
#include <QSqlTableModel>
#include <QDesktopServices>

TableResult::TableResult(ParoutModel *pm, QWidget *parent) :
    QStandardItemModel(pm), pm(pm), parentWidget(parent)
{
    assert(pm);
    tableCalculator = new TableCalculator(this);
}

int TableResult::parameterColumnCount() const
{ return pm->parameterModel()->columnCount(); }

QVariant TableResult::data(const QModelIndex &idx, int role) const
{
    if(! hasIndex(idx.row(),idx.column()) )
        return QVariant();

    QStandardItem *hItem = horizontalHeaderItem(idx.column());
    QStandardItem *vItem = verticalHeaderItem(idx.row());

    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if( !vItem) return QVariant();
        if (vItem->data(Item::DataRole).toInt() != Item::OtherRow)\
        {
            if( !hItem ) return QVariant();

            switch(hItem->data(Item::HeaderFlagRole).toInt() )
            {
            case Item::Parameter:
            case Item::Parafiter:
            case Item::Id:
                assert(pm->parameterModel());
                return pm->parameterModel()->data(idx,role);
            default:
                return QStandardItemModel::data(idx, role);
            }
        }
        else
        {
            if(!hItem) return "-";

            switch(hItem->data(Item::HeaderFlagRole).toInt() )
            {
            case Item::Parafiter:
                return hItem->data(Item::DataRole);
            case Item::Parameter:
            case Item::Id:
                return QStandardItemModel::data(idx, Qt::DisplayRole);
            default:
                return "-";
            }
        }
    case Qt::BackgroundRole:
        if(!vItem) return QVariant();

        if (vItem->data(Item::DataRole).toInt() == Item::OtherRow)
            return QBrush (QColor(Qt::lightGray));
        else if( hItem && hItem->data(Item::HeaderFlagRole).toInt() & (Item::Parafiter|Item::Parameter))
            return QBrush (QColor(Qt::white));
        else
            return QVariant();
    case Qt::ForegroundRole:
        if( !hItem || !vItem) return QVariant();

        switch(hItem->data(Item::HeaderFlagRole).toInt() )
        {
        case Item::Parafiter:
        case Item::Id:
            return QBrush (QColor(Qt::darkMagenta));
        case Item::Parameter:
            return vItem->data(Item::DataRole).toInt() == Item::SimData ?
                        QBrush (QColor(Qt::black)) :
                        QBrush (QColor(Qt::darkGreen));
        case Item::Times:
            return QBrush (QColor(Qt::blue));
        default:
            return QBrush (QColor(Qt::black));
        }
    default:
        return QStandardItemModel::data(idx, role);
    }
}

Qt::ItemFlags TableResult::flags(const QModelIndex &index) const
{    
    if(!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags fgs = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    QStandardItem *hItem = horizontalHeaderItem(index.column());
    QStandardItem *vItem = verticalHeaderItem(index.row());
    if( !hItem || !vItem)
        return fgs;

    if( (hItem->data(Item::HeaderFlagRole).toInt() ==  Item::Times && vItem->data(Item::DataRole).toInt() != Item::OtherRow) ||
            (hItem->data(Item::HeaderFlagRole).toInt() ==  Item::Parameter && vItem->data(Item::DataRole).toInt() != Item::SimData ) )
        fgs |= Qt::ItemIsEditable;

    return fgs;
}

bool TableResult::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role != Qt::EditRole)
        return QStandardItemModel::setData(index,value,role);

    QStandardItem *hItem = horizontalHeaderItem(index.column());
    QStandardItem *vItem = verticalHeaderItem(index.row());
    if( !hItem || !vItem)
        return false;

    switch(hItem->data(Item::HeaderFlagRole).toInt() )
    {
    case Item::Parameter:
        switch(vItem->data(Item::DataRole).toInt() )
        {
        case Item::OtherRow:
        {
            bool ok = QStandardItemModel::setData(index,value,Qt::EditRole);
            if( value.isValid())
                ok &= submitLastRow();
            return ok;
        }
        case Item::NoSimData:
            if(! value.isValid())
                return false;
            assert(pm->parameterModel());
            if(! pm->parameterModel()->setData(pm->parameterModel()->index(index.row(),index.column()),value,Qt::EditRole))
                return false;
            if( ! QStandardItemModel::setData(index,value,Qt::EditRole))
                return false;
            calculateRow(index.row());
            return true;
        default:
            return false;
        }
    case Item::Parafiter:
        return false;
    default:
        return QStandardItemModel::setData(index,value,Qt::EditRole);
    }
}

bool TableResult::submitLastRow()
{
    assert(pm->parameterModel());
    assert (pm->parameterModel()->rowCount() == this->rowCount()-1);

    int lastRow = this->rowCount()-1;

    QSqlRecord record = pm->parameterModel()->record();
    for(int j=1; j<record.count(); ++j)
    {
        QVariant val = data(index(lastRow,j));
        if(val.isNull())
            return false;
        record.setValue(j,val);
    }

    if (! pm->parameterModel()->insertRecord(-1,record)) //appended to the end.
        return false;

    if (! pm->parameterModel()->select())
        return false;

    QString newkey = pm->parameterModel()->index(lastRow,0).data().toString();
    if(   pm->existSimulation(newkey) )
    {
        int answer = QMessageBox::warning(parentWidget,tr("Warning"),"Unknown simulation datas of the key <b>" + newkey + "</b> exists",QMessageBox::Discard|QMessageBox::Save,QMessageBox::Discard) ;
        if( answer == QMessageBox::Discard)
            pm->removeKey(newkey);
    }

    this->setVerticalHeaderItem(  lastRow, pm->newKeyItem( newkey) );
    this->calculateRow( lastRow); //calculate post region's data of the new ith row

    if (! lastRow)
        emit pm->simulationExistence(true);

    ++lastRow;
    QStandardItemModel::insertRow( lastRow);
    QStandardItemModel::setVerticalHeaderItem( lastRow, pm->newLastRowItem());
    return true;
}

void TableResult::refreshAll()
{
    for (int row=0; row< rowCount()-1; ++row)
    {
        QStandardItem *item = verticalHeaderItem(row);
        if(! item || ! item->text().contains(QRegExp("^\\s*\\d+\\s*$")) )
            continue;

        bool exist = pm->existSimulation(item->text());

        item->setData( exist ? Item::SimData : Item::NoSimData, Item::DataRole);

        QFont font = item->font();
        font.setItalic(exist);
        item->setFont(font);
    }
}

int TableResult::columnOf(const QString &header) const
{
    if(header.isEmpty())
        return -1;

    for(int section=0; section<columnCount(); ++section)
        if(headerData(section,Qt::Horizontal).toString()==header)
            return section;
    return -1;
}

bool TableResult::isFile(const QModelIndex &idx) const
{
    QStandardItem *hItem = horizontalHeaderItem(idx.column());
    QStandardItem *vItem = verticalHeaderItem(idx.row());
    if(     ( ! vItem || ! (vItem->data(Item::DataRole).toInt() & (Item::SimData|Item::NoSimData) ) )  ||
            ( ! hItem || ! (hItem->data(Item::HeaderFlagRole).toInt() & (Item::FileOt | Item::FileSt) ) ))
        return false;
    return true;
}

QString TableResult::directoryPath(const QModelIndex &idx) const
{
    QStandardItem *hItem = horizontalHeaderItem(idx.column());
    QStandardItem *vItem = verticalHeaderItem(idx.row());
    if( ! vItem || ! hItem || ! (vItem->data(Item::DataRole).toInt() & (Item::SimData|Item::NoSimData))  )
        return QString();

    switch (hItem->data(Item::HeaderFlagRole).toInt()) {
    case Item::FileSt:
    {
        const QString path = pm->filePath(vItem->text(),hItem->text());
        if(path.isEmpty() )
            return path;
        return QFileInfo(path).absolutePath();
    }
    case Item::FileOt:
    {
        const QStringList paths = pm->filePathList(vItem->text(),hItem->text());
        if(paths.isEmpty())
            return QString();
        return QFileInfo(paths.first()).absolutePath();

    }
    default:
        return QString();
    }
}

std::vector<double> TableResult::data(int row, const QString &header) const
{
    int column = columnOf(header);

    QStandardItem *hItem = horizontalHeaderItem(column);
    QStandardItem *vItem = verticalHeaderItem(row);

    if( !hItem || (!vItem || vItem->data(Item::DataRole).toInt() == Item::OtherRow) )
        return std::vector<double>();

    switch(hItem->data(Item::HeaderFlagRole).toInt())
    {
    case Item::Parameter:
    case Item::Parafiter:
    case Item::Formula:
        return std::vector<double>(1,data(index(row,column)).toDouble());
    case Item::FileSt:
        return pm->readStFile(vItem->text(),hItem->text());
    default:
        return std::vector<double>();
    }
}

QString TableResult::dataForCopy(const QModelIndex &index) const
{
    QStandardItem *hItem = horizontalHeaderItem(index.column());
    QStandardItem *vItem = verticalHeaderItem(index.row());

    if( (!hItem || hItem->data(Item::HeaderFlagRole).toInt()==Item::Times) || (!vItem || vItem->data(Item::DataRole).toInt() == Item::OtherRow) )
        return QString();

    switch(hItem->data(Item::HeaderFlagRole).toInt())
    {
    case Item::Parameter:
    case Item::Parafiter:
    case Item::Formula:
        return data(index).toString();
    case Item::FileSt:
    {
        QString path = pm->filePath(vItem->text(),hItem->text());
        return path.append('\'').prepend('\'');
    }
    case Item::FileOt:
    {
        QStringList list = pm->filePathList(vItem->text(),hItem->text());
        for (int i= 0; i < list.count(); ++i)
            list[i].append('\'').prepend('\'');
        return list.join(',').prepend('{').append('}');
    }
    default:
        return QString();
    }
}

QString TableResult::headerDataForCopy(int column) const
{
    QStandardItem *hItem = horizontalHeaderItem(column);

    if(! hItem)
        return QString();

    QString name = hItem->text().trimmed();
    if(! name.replace('.','_').contains(QRegExp("^[a-zA-Z_]\\w*$")) )
        return QString();

    switch(hItem->data(Item::HeaderFlagRole).toInt())
    {
    case Item::Parafiter:
    case Item::Parameter:
    case Item::Formula:
        return name+"=[";
    case Item::FileOt:
    case Item::FileSt:
        return name+"={";
    default:
        return QString();
    }
}

//读取所有
void TableResult::calculate(int row, int column)
{
    QStandardItem *hItem = horizontalHeaderItem(column);
    QStandardItem *vItem = verticalHeaderItem(row);
    if( !hItem || (!vItem || vItem->data(Item::DataRole).toInt() == Item::OtherRow) )
    {
        setData(index(row,column),QVariant());
        return;
    }

    switch(hItem->data(Item::HeaderFlagRole).toInt())
    {
    case Item::Formula:
        setData( index(row,column), tableCalculator->caculate(row,hItem->data(Item::DataRole).toString()) );
        break;
    case Item::FileSt:
        setData( index(row,column), pm->stRecordCount(vItem->text(),hItem->text() ) );
        break;
    case Item::FileOt:
        setData( index(row,column), pm->dyRecordCount(vItem->text(),hItem->text() ) );
        break;
    }
}

bool TableResult::removeColumns(int column, int count, const QModelIndex &parent)
{
    column += count; // the end column.
    int i=0;
    for (; i < count; ++i)
    {
        if(horizontalHeaderItem(column-1)) //previous column
            break;
        --column;
    }
    if(! i) return false;
    return QStandardItemModel::removeColumns(column, i, parent);
}

bool  TableResult::setHeaderText(int section, const QString &value)
{
    if(section <= parameterColumnCount())
        return false;
    
    QString text = value.trimmed();

    if(text.isEmpty())
    {
        QStandardItemModel::setHorizontalHeaderItem(section,0);
        calculateColumn(section);
        return true;
    }

    QString name = text;
    QRegExp fm("^([a-zA-Z_]\\w*)\\s*=");
    if( text.contains(fm) )
        name = fm.cap(1);
    for(int i = 0; i < columnCount(); ++i)
    {
        QStandardItem  *hItem = horizontalHeaderItem(i);

        if(i == section || ! hItem)
            continue;
        if(hItem->text().toLower() == name.toLower() )
        {
            QToolTip::showText( QCursor::pos(),"<b>" + text + "</b> has already been used or is a preserve word!");
            return false;
        }
    }

    if( text == headerText(section) )
        return false;

    QStandardItem *item = pm->newFileformulaItem(text);
    QStandardItemModel::setHorizontalHeaderItem(section,item);
    calculateColumn(section);
    return true;
}

QString TableResult::headerText(int section) const
{
   QStandardItem * item = horizontalHeaderItem(section);
   if(! item) return QString();

   if(item->data(Item::HeaderFlagRole).toInt() != Item::Formula)
        return item->text();

    return item->data( Qt::ToolTipRole).toString();
}

bool TableResult::removeRows(int row, int count, const QModelIndex &parent)
{
    if(count != 1 )
        return false;

    QStandardItem *vItem = verticalHeaderItem(row);
    assert(vItem);
    switch(vItem->data(Item::DataRole).toInt())
    {
    case Item::SimData:
    {
        int answer = QMessageBox::warning(parentWidget,tr("Warning"), "Simulation datas to the row <b>"+vItem->text() + "</b> exist.", QMessageBox::Discard|QMessageBox::Cancel, QMessageBox::Cancel);
        if(answer == QMessageBox::Cancel)
            return false;
        if(! pm->removeKey(vItem->text()))
        {
            QMessageBox::critical(parentWidget,tr("Error"),tr("Failed to clear the simulation datas.\n\nPlease clear them manually."),QMessageBox::Ok | QMessageBox::Ok);
            QDesktopServices::openUrl(QUrl::fromLocalFile( pm->keyPath(vItem->text())));
        }
    }
    case Item::NoSimData:
    {
        assert(pm->parameterModel());
        if(! pm->parameterModel()->removeRows(row,count) )
            return false;
        pm->parameterModel()->select();
        if(pm->parameterModel()->rowCount() == 0)
            emit pm->simulationExistence(false);
        return QStandardItemModel::removeRows(row,count,parent);
    }
    default:
        return false;
    }
}
