/** @file editorobjmodel.h
* @brief the C++ object model.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the C++ object model Class.
*/

#ifndef EDITOROBJMODEL_H
#define EDITOROBJMODEL_H

#include <QAbstractItemModel>
#include <QVector>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QStandardItem;
QT_END_NAMESPACE
class FunctionItem;

class CppObjectModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CppObjectModel(QObject *parent = 0);
    QModelIndex index(int row, int column = 1, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex & = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QStandardItem *itemOf(const QString &name, const QString &scope) const;
    QStandardItem * itemOf(const QString &path) const;

    void prependModel(QStandardItemModel *model);
    void appendPersistentModel(QStandardItemModel *model);
    void clear();
private:
    QStandardItem *itemFromIndex( const QModelIndex& index) const
    { return static_cast<QStandardItem*>(index.internalPointer()); }
    QModelIndex findChild(const QString &name, const QModelIndex &parent) const;
    int rowOf(QStandardItemModel *model) const;
private:
    QVector<QStandardItemModel *> models;
    int perscount;
};

#endif // EDITOROBJMODEL_H
