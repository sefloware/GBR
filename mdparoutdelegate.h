/** @file mdparoutdelegate.h
* @brief the styled item delegate for the model of the parout Class.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the styled item delegate Class for combobox editors.
*/

#ifndef MDPAROUTDELEGATE_H
#define MDPAROUTDELEGATE_H

#include <QStyledItemDelegate>
#include <QSqlTableModel>
#include <QLineEdit>
#include <QComboBox>
#include "mdparout.h"

class SortedDoubleListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SortedDoubleListModel(QObject *parent = 0) : QAbstractListModel(parent) {}
    int rowCount(const QModelIndex &/*parent*/ = QModelIndex()) const
    { return list.count(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if( index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole) )
            return list.at(index.row());
        return QVariant();
    }

    void sort(int /*column*/ = 0, Qt::SortOrder order = Qt::AscendingOrder)
    {
        if(order == Qt::AscendingOrder)
            qSort( list);
        else
            qSort( list.begin(), list.end(), qGreater<double>() );
    }

    void insert(double val)
    {
        if(! list.contains(val))
            list.append(val);
    }

private:
    QList<double> list;
};

class ParoutDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ParoutDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
        const QModelIndex &index) const
    {
        if(! index.model() || index.column() != 1 )
            return 0;

        const ParoutModel *model = static_cast<const ParoutModel *>( index.model() );
        QString fieldName = model->index(index.row(),0).data().toString();
        int col = model->parameterModel()->fieldIndex(fieldName);
        if(col==-1)
            return 0;   

        QComboBox *editor = new QComboBox(parent);
        SortedDoubleListModel *doubleList = new SortedDoubleListModel(editor);
        for(int i=0; i<model->parameterModel()->rowCount(); ++i)
            doubleList->insert( model->parameterModel()->index(i,col).data().toDouble());
        doubleList->sort();

        editor->setModel(doubleList);
        editor->setEditable(true);
        editor->setValidator( new QDoubleValidator(editor));

        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QComboBox *box = static_cast<QComboBox*>(editor);
        box->setCurrentText(index.data().toString() );
        if(! box->lineEdit()->hasAcceptableInput())
            box->clearEditText();
        else
            box->lineEdit()->selectAll();
        box->lineEdit()->setFocus();
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
    {
        QComboBox *box = static_cast<QComboBox*>( editor);
        const QString val = box->currentText();
        if( box->lineEdit()->hasAcceptableInput())
            model->setData(index, val);
        else if ( val.isEmpty() )
            model->setData(index, QVariant() );
    }

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &) const
    { editor->setGeometry(option.rect); }
};

#endif // MDPAROUTDELEGATE_H
