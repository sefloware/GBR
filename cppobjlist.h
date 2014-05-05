/** @file cppobjlist.h
* @brief Parsing the C++ objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the C++ objects, which
* is the routine of the C++ autocompleter and
* syntax highlighter.
*/

#ifndef CPPOBJLIST_H
#define CPPOBJLIST_H

#include <QList>

QT_BEGIN_NAMESPACE
class QStandardItem;
class QStringList;
class QTextStream;
QT_END_NAMESPACE
class CppReader;
class CppStructReader;

class CppObjList : public QList<QStandardItem *>
{
public:
    //search item of name.
    void readDescription(const QString &text);
    QStandardItem *indexOf(const QString &text) const;
    static QStandardItem *findSubitem(const QStringList &args,const QList<QStandardItem *> &subitems);
    static QStandardItem *findSubitem(const QStringList &args, int flag,const QList<QStandardItem *> &subitems);
    //transform CppReader to CppObjList;
    CppObjList &operator<<(CppReader &cr);
    CppObjList &operator<<(CppStructReader &csr);
private:
    void readDescription(const QList<QStandardItem *> &itemList, QTextStream &in);
};

#endif // CPPOBJLIST_H
