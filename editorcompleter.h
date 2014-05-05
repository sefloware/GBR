/** @file editorcompleter.h
* @brief the C++ code completer Class.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the C++ code completer Class.
*/

#ifndef EDITORCOMPLETER_H
#define EDITORCOMPLETER_H

#include <QCompleter>
#include <QStringList>

class CodeCompleter : public QCompleter
{
    Q_OBJECT
    Q_PROPERTY(QString separator READ separator WRITE setSeparator)

public:
    explicit CodeCompleter(QObject *parent = 0)
        : QCompleter(parent) {}
    explicit CodeCompleter(QAbstractItemModel *model, QObject *parent = 0)
        : QCompleter(model, parent) {}

    QString separator() const
    { return sep; }
    void setSeparator(const QString &separator)
    { sep = separator;}
protected:
    QStringList splitPath(const QString &path) const
    {
        if (sep.isNull()) {
            return QCompleter::splitPath(path);
        }
        return path.split(sep);
    }
    QString pathFromIndex(const QModelIndex &index) const
    {
        if (sep.isNull()) {
            return QCompleter::pathFromIndex(index);
        }

        // navigate up and accumulate data
        QStringList dataList;
        for (QModelIndex i = index; i.isValid(); i = i.parent()) {
            dataList.prepend(model()->data(i, completionRole()).toString());
        }

        return dataList.join(sep);
    }
private:
    QString sep;
};

#endif // EDITORCOMPLETER_H

