/** @file table.h
* @brief the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the data table.
*/

#ifndef TABLE_H
#define TABLE_H

#include <QStandardItemModel>
#include <vector>

class TableCalculator;
class ParoutModel;

class TableResult : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit TableResult(ParoutModel *pm,QWidget *parent = 0);
    int parameterColumnCount() const;

    //QStandardItemModel Virtual.
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    
    bool setHeaderText(int section,const QString &value);
    QString headerText(int section) const;
    
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const;
    std::vector<double> data(int row, const QString &header) const; //calculator

    QString dataForCopy(const QModelIndex &index) const;
    QString headerDataForCopy(int column) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QString directoryPath(const QModelIndex &idx) const;
    bool isFile(const QModelIndex &idx) const;
public slots:
    bool submitLastRow();
    void calculateAll() {
        for (int j=parameterColumnCount() + 1; j<columnCount(); ++j)
            calculateColumn(j);
    }
    void refreshAll();
    void calculate(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        for(int i=topLeft.row(); i<= bottomRight.row(); ++i)
            calculateRow(i);
    }
private:
    void calculateColumn(int column) {
        for (int i=0; i<rowCount(); ++i)
            calculate(i,column);
    }
    void calculateRow(int row) {
        for (int j=parameterColumnCount(); j<columnCount(); ++j)
            calculate(row,j);
    }
    void calculate(int row,int column);

    int columnOf(const QString &header) const;
private:
    TableCalculator *tableCalculator;
    ParoutModel *pm;
    QWidget *parentWidget;

    friend class TableFilter;
};

#endif // TABLE_H
