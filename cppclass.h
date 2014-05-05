/** @file cppclass.h
* @brief Parsing the Class objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the Class objects, which
* is the subroutine of the C++ autocompleter and
* syntax highlighter.
*/

#ifndef CPPCLASS_H
#define CPPCLASS_H
#include "item.h"

class CppObjList;
class CppReader;

class ClassItem : public Item::CppItem
{
public:
    ClassItem() : Item::CppItem(Item::Class) {}
    ~ClassItem();
    QVariant data(int role = Qt::UserRole+1) const;
    QString typeOf(const QString &) const;
    ClassItem *clone() const;
private:
    QStringList description() const;
    //match for the overload operator []
    QStandardItem *findSquareOperator(const QString &arg) const;
    //match the overload operator ()
    QMultiMap<int, QStandardItem *> findRoundOperators(const QStringList &arg, bool exactly) const;
public:
    QList<QStandardItem *> subitems; //the storage of the overload operator.
};

class PathClass
{
    //try to make the reader pass the class path.
    //If done, it will record the context of the reader into objs as a ClassItem.
public:
    PathClass(const CppReader &reader, CppObjList &objs);
    operator bool() const { return ok; }
private:
    ClassItem *formItem(const QString &name) const;
private:
    bool ok;
    const CppReader &reader;
};

#endif // CPPCLASS_H
