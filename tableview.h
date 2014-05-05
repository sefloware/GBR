/** @file tableview.h
* @brief the view of the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the view of the data table.
*/

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include "tablefilter.h"

QT_BEGIN_NAMESPACE
class QShortcut;
QT_END_NAMESPACE

class TableView : public QTableView
{
    Q_OBJECT
public:
    explicit TableView(QWidget *parent = 0);

    void setModel(TableFilter *model)
    { QTableView::setModel(model); }
    TableFilter *model() const
    { return static_cast<TableFilter *>(QTableView::model()); }
    TableResult *sourceModel() const
    { return model()->sourceModel(); }
public slots:
    void showTimes(bool show);
    void initializeApperance();
private slots:
    void resetColumnCount(int value);
    void resetColumnCount();
    void addFilter();
    void deleteFilter();
    void deleteRow();
    void clear();
    void copy();
    void copyForMatlab();
    void explore();
    bool del();
private:
    QString construct(QString name, const QStringList &valist);
protected:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    void resizeEvent(QResizeEvent *event);
private:
    enum {DefaultItemSize = 90};
    bool showtimes;
    QShortcut *m_del;
public:
    QAction *addfilterAction;
    QAction *delfilterAction;
    QAction *delrowAction;
    QAction *delAction;
    QAction *copyAction;
    QAction *copyContentAction;
    QAction *exploreAction;
private:
    struct RowLess
    {
        bool operator() (const QModelIndex &index1,const QModelIndex &index2)
        { return (index1.row() < index2.row()) || (index1.row() == index2.row() && index1.column() < index2.column() ); }
    };

    struct ColumnLess
    {
        bool operator() (const QModelIndex &index1,const QModelIndex &index2)
        { return (index1.column() < index2.column()) || (index1.column() == index2.column() && index1.row() < index2.row() ); }
    };
};

#endif // TABLEVIEW_H
