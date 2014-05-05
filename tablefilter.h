/** @file tablefilter.h
* @brief the data filter of the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the data filter of the data table.
*/

#ifndef TABLEFILTER_H
#define TABLEFILTER_H

#include <QSortFilterProxyModel>
#include <QSqlTableModel>
#include "assert.h"

class TableResult;

class TableFilter : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    TableFilter(QObject *parent = 0);
    TableResult *sourceModel() const;
    void setSourceModel(TableResult *sourceModel);
    void invalidateFilter();
    int parameterColumnCount() const;
    bool needHide(int section) const;
    QString headerDataForCopy(int column) const;
    QString dataForCopy(const QModelIndex &idx) const;
    bool isFile(const QModelIndex &idx) const;
    QString directoryPath(const QModelIndex &idx) const;

    bool removeFile(const QString &key, const QString &fileName) const;
    int generateMain(const QString &mainsource, const QString &path, QStringList &idList, QWidget *parent = 0) const;
public slots:
    void addFilter(const QModelIndex &index);
    void deleteFilter(const QModelIndex &index);

    void addFilter(int section, const QVariant &val);
    void deleteFilter(int section);
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &/*source_parent*/) const;
    bool lessThan(const QModelIndex & left, const QModelIndex & right) const;
    QString cmainOf(int row,QString omain) const;
    bool isInOtherRow(const QModelIndex &index) const;
};

#endif // TABLEFILTER_H
