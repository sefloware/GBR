/** @file tabledelegate.h
* @brief the item delegate of the data table.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the item delegate of the data table.
*/

#ifndef TABLEDELEGATE_H
#define TABLEDELEGATE_H

#include <QStyledItemDelegate>
#include <QLineEdit>

class ResultDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ResultDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
        const QModelIndex &index) const
    {
        if(! index.model())
            return 0;

        QLineEdit *editor = new QLineEdit(parent);
        editor->setValidator( new QDoubleValidator(editor));
        editor->setToolTip("Please input <b>double</b> data!");
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(index.data().toString() );
        lineEdit->selectAll();
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>( editor);

        QVariant value;
        if( lineEdit->hasAcceptableInput() )
            value = lineEdit->text();

        model->setData(index, value );
    }

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &) const
    { editor->setGeometry(option.rect); }
};

#endif // TABLEDELEGATE_H
