/** @file cppfunction.h
* @brief Parsing the Function objects and Operator objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the Function objects and
* Operator objects, which is the subroutine of the C++
* autocompleter and syntax highlighter.
*/

#ifndef CPPFUNCTION_H
#define CPPFUNCTION_H
#include "item.h"
class CppStructReader;
class CppReader;
class CppObjList;

class FunctionItem : public Item::CppItem
{
public:
    FunctionItem() : Item::CppItem(Item::Function) {}
    ~FunctionItem();
    QVariant data(int role = Qt::UserRole+1) const;
    QString typeOf(const QString &str) const;
    FunctionItem *clone() const;
private:
    QStringList description() const;
    QMultiMap<int, QStandardItem *> matchArgument(const QStringList &arg,bool exactly = true) const;
public:
    QList<QStandardItem *> subitems; //the storage of the overload function.
};

class PathFunction
{
public:
    PathFunction(const CppReader &reader,CppObjList &objs);
    operator bool() const { return ok; }
private:
    FunctionItem *formNameItem() const;
    QStandardItem *formOverloadItem() const;
private:
    bool ok;
    QString type;
    QString name;
    QStringList argtypes;

    const CppReader &reader;
};

struct PathOperator
{
public:
    PathOperator(CppStructReader &reader);
    operator bool() const { return ok; }
private:
    template <Item::CppTypeFlags flag>
    QStandardItem *formOpitem() const;
private:
    bool ok;
    QString type;
    QStringList argtypes;
    CppStructReader &reader;
};

#endif // CPPFUNCTION_H
