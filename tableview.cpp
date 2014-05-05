/** @file tableview.cpp
* @brief the view of the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the view of the data table.
*/

#include "tableview.h"
#include "tablefilter.h"
#include "table.h"
#include "item.h"
#include "assert.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QVariant>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QSqlTableModel>
#include <QScrollBar>
#include <QDesktopServices>
#include <QFileInfo>
#include <QShortcut>
#include <QToolTip>

TableView::TableView(QWidget *parent) :
    QTableView(parent),showtimes(false)
{
    addfilterAction = new QAction(tr("&Add filter"),this);
    addfilterAction->setIcon(QIcon(":/icon/images/addfilter.png"));
    addfilterAction->setEnabled(false);
    connect(addfilterAction,SIGNAL(triggered()), this,SLOT(addFilter()) );

    delfilterAction = new QAction(tr("&Del filter"),this);
    delfilterAction->setIcon(QIcon(":/icon/images/deletefilter.png"));
    delfilterAction->setEnabled(false);
    connect(delfilterAction,SIGNAL(triggered()), this,SLOT(deleteFilter()) );

    delrowAction = new QAction(tr("&Del row"),this);
    delrowAction->setEnabled(false);
    delrowAction->setIcon(QIcon(":/icon/images/deleterow.png"));
    connect(delrowAction,SIGNAL(triggered()), this,SLOT(deleteRow()) );

    delAction = new QAction(tr("&Del"),this);
    delAction->setEnabled(false);
    delAction->setIcon(QIcon(":/icon/images/delete.png"));
    connect(delAction,SIGNAL(triggered()), this,SLOT(clear()) );

    copyAction = new QAction(tr("&Copy"),this);
    copyAction->setEnabled(false);
    copyAction->setIcon(QIcon(":/icon/images/copy.png"));
    connect(copyAction,SIGNAL(triggered()), this,SLOT(copy()) );

    copyContentAction = new QAction(tr("&Copy for Matlab"),this);
    copyContentAction->setEnabled(false);
    copyContentAction->setIcon(QIcon(":/icon/images/copyformatlab.png"));
    connect(copyContentAction,SIGNAL(triggered()), this,SLOT(copyForMatlab()) );

    exploreAction = new QAction(tr("&Explore the output in the folder"),this);
    exploreAction->setEnabled(false);
    exploreAction->setIcon(QIcon(":/icon/images/explore.png"));
    connect(exploreAction,SIGNAL(triggered()), this,SLOT(explore()) );

    m_del = new QShortcut(QKeySequence(QKeySequence::Delete),this);
    connect(m_del,SIGNAL(activated()),this,SLOT(del()) );

    this->addAction(addfilterAction);
    this->addAction(delfilterAction);
    this->addAction(delrowAction);
    this->addAction(copyAction);
    this->addAction(copyContentAction);
    this->addAction(exploreAction);
    this->setContextMenuPolicy(Qt::ActionsContextMenu);

    horizontalHeader()->setMinimumSectionSize(DefaultItemSize);
    horizontalHeader()->setDefaultSectionSize(DefaultItemSize);

    connect(this->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(resetColumnCount(int)) );
}

bool TableView::del()
{
    QModelIndex index = currentIndex();

    int hFlag = model()->headerData(index.column(),Qt::Horizontal,Item::HeaderFlagRole).toInt();
    int vFlag = model()->headerData(index.row(),Qt::Vertical,Item::DataRole).toInt();
    if(     ( hFlag == Item::Parameter && vFlag == Item::OtherRow) ||
            ( hFlag == Item::Times && (vFlag & (Item::SimData | Item::NoSimData ))) )
        return model()->setData(index,QVariant());

    if(     ( hFlag & (Item::FileOt | Item::FileSt) ) && (vFlag & (Item::SimData | Item::NoSimData)))
    {
        const QString key = model()->headerData(index.row(),Qt::Vertical,Qt::DisplayRole).toString();
        const QString fileName = model()->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString();
        if(! model()->removeFile(key,fileName) )
        {
            QToolTip::showText(QCursor::pos(),tr("Failed to clear datas!"));
            return false;
        }
        return true;
    }

    return false;
}

void TableView::resetColumnCount(int value)
{
    assert(model());

    static int preval = 0;
    if(preval > value)
    {
        preval = value;
        int col = columnAt(viewport()->width());
        if (col == -1)
            return;
        int count = model()->columnCount() - (++col);
        if(count < 2) return;
        model() ->removeColumns(col,count);
    }
    preval = value;
}

void TableView::resetColumnCount()
{
    if(! model())
        return;

    int width = viewport()->width();
    int lastsectionposition = horizontalHeader()->sectionPosition(model()->columnCount() -1);
    int dist = width - lastsectionposition;
    int lastsectionwidth = horizontalHeader()->sectionSize(model()->columnCount() -1);
    if(dist > lastsectionwidth )
    {
        model()->insertColumns(model()->columnCount(),dist/DefaultItemSize + 1);
    }
    else
    {
        int col = columnAt(width);
        assert(col != -1);
        col += 2;
        const int count = model()->columnCount() - col;
        if( count > 1)
            model() ->removeColumns(col,count);
    }
}


QModelIndex TableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    assert(model());

    if((cursorAction == QAbstractItemView::MoveRight && ! modifiers) &&
        (currentIndex().column() == model()->columnCount()-1) )
        model()->insertColumn( model()->columnCount());

    return QTableView::moveCursor(cursorAction,modifiers);
}

void TableView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    resetColumnCount();
}

void TableView::showTimes(bool show)
{
    showtimes = show;

    int section = model()->parameterColumnCount();

    if(section)
    {
        if(show)    showColumn(section );
        else        hideColumn(section );
    }
}
void TableView::initializeApperance()
{
    if(model()->parameterColumnCount())
         hideColumn(0);

    for(int i=1; i< model()->parameterColumnCount(); ++i)
    {
        if(model()->needHide(i))    hideColumn(i);
        else                        showColumn(i);
    }

    showTimes(showtimes);
}

void TableView::clear()
{
    assert(model());

    QModelIndexList idxs = selectedIndexes();
    foreach (const QModelIndex &it, idxs) {
        model()->setData(it,QVariant());
    }
}

void TableView::copy()
{
    QModelIndexList indexes = this->selectedIndexes();
    assert(! indexes.isEmpty());

    qSort(indexes.begin(),indexes.end(),RowLess());
    QString selected_text;
    // You need a pair of indexes to find the row changes
    QModelIndex previous = indexes.takeFirst();
    selected_text.append(previous.data().toString());
    foreach (const QModelIndex &it, indexes)
    {
        QString text = it.data().toString();
        if (it.row() != previous.row())
            selected_text.append( QString( it.row()-previous.row(),'\n') );
        else
            selected_text.append( QString( it.column()-previous.column(),'\t'));

        selected_text.append(text);
        previous = it;
    }
    QApplication::clipboard()->setText(selected_text);
}

QString TableView::construct(QString name, const QStringList &valist)
{
    if(name.isEmpty() || valist.isEmpty())
        return QString();
    else if(valist.count() == 1)
        name.chop(1);

    if(name.endsWith('['))
        return name + valist.join(',') + "];";
    else if (name.endsWith('{'))
        return name + valist.join(',') + "};";
    else
        return name + valist.join(',') + ";";
}

void TableView::copyForMatlab()
{
    QModelIndexList indexes = this->selectedIndexes();
    assert(! indexes.isEmpty());
    qSort(indexes.begin(),indexes.end(),ColumnLess());

    QStringList list;
    int column = -1;
    QStringList valist;
    QString name;
    foreach (const QModelIndex &it, indexes)
    {
        if(column != it.column())
        {
            //save the previous record.
            QString tmp = construct(name, valist);
            if(! tmp.isEmpty())
                list << tmp;

            //init
            column = it.column();  
            name = model()->headerDataForCopy(column);
            valist.clear();
        }

        if(name.isEmpty())
            continue;

        QString val = model()->dataForCopy(it);
        if(val.isEmpty())
            continue;

        valist << val;
    }

    //save the current record.
    QString tmp = construct(name, valist);
    if(! tmp.isEmpty())
        list << tmp;

    QApplication::clipboard()->setText(list.join('\n'));
}

void TableView::explore()
{
    assert(model());
    QString path = model()->directoryPath(currentIndex());
    if(! path.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile( path));
}

void TableView::addFilter()
{
    assert(model());

    model()->addFilter(currentIndex());

    addfilterAction->setEnabled(false);
    delfilterAction->setEnabled(true);
    initializeApperance();
}

void TableView::deleteFilter()
{
    assert(model());

    model()->deleteFilter(currentIndex());

    addfilterAction->setEnabled(true);
    delfilterAction->setEnabled(false);
    initializeApperance();
}

void TableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTableView::selectionChanged(selected,deselected);

    if(! model()) return;

    QModelIndexList idxs = selected.indexes();
    if(idxs.count() != 1 )
        addfilterAction->setEnabled(false);
    else
    {
        bool enable =  model()->headerData(idxs.first().column(),Qt::Horizontal,Item::HeaderFlagRole).toInt() & (Item::Parameter | Item::Parafiter) &&
                model()->headerData(idxs.first().row(),Qt::Vertical,Item::DataRole).toInt() != Item::OtherRow;
        addfilterAction->setEnabled(enable);
    }

    QSet<int> set;
    foreach (const QModelIndex &it, idxs) {
        set.insert(it.column());
    }
    if(set.count() != 1)
        delfilterAction->setEnabled(false);
    else
    {
        bool enable =  model()->headerData(*set.begin(),Qt::Horizontal,Item::HeaderFlagRole).toInt() == Item::Parafiter;
        delfilterAction->setEnabled(enable);
    }

    set.clear();
    foreach (const QModelIndex &it, idxs) {
        set.insert(it.row());
    }
    if(set.count() != 1)
        delrowAction->setEnabled(false);
    else
    {
        bool enable = selected.contains(model()->index(*set.begin(),0));
        delrowAction->setEnabled(enable);
    }

    copyAction->setEnabled(idxs.count());
    copyContentAction->setEnabled(idxs.count());

    assert(model());
    exploreAction->setEnabled(model()->isFile(currentIndex()));

    if(idxs.count() != 1)
        m_del->setEnabled(false);
    else
    {
        int hFlag = model()->headerData(idxs.first().column(),Qt::Horizontal,Item::HeaderFlagRole).toInt();
        int vFlag = model()->headerData(idxs.first().row(),Qt::Vertical,Item::DataRole).toInt();
        bool enable = ( hFlag == Item::Parameter) && (vFlag == Item::OtherRow) ||
                ( hFlag & (Item::Times | Item::FileOt | Item::FileSt) && (vFlag & (Item::SimData | Item::NoSimData)));
        m_del->setEnabled(enable);
    }
}

void TableView::deleteRow()
{
    assert(model());

    QModelIndex idx = currentIndex();
    if(! idx.isValid())
        return;
    model()->removeRow(idx.row());
}

