/** @file cppenumespace.h
* @brief Parsing the Enum objects and the Namespace objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the Enum objects and the
* Namespace objects, which is the subroutine of the
* C++ autocompleter and syntax highlighter.
*/

#ifndef CPPENUMESPACE_H
#define CPPENUMESPACE_H
#include "item.h"

class CppReader;
class CppObjList;

class PathEnum
{
public:
    PathEnum(const CppReader &reader,CppObjList &objs);
    operator bool() const { return ok; }
private:
    bool ok;
    const CppReader &reader;
    QString name;
};

class PathNamespace
{
public:
    PathNamespace(const CppReader &reader,CppObjList &objs);
    operator bool() const { return ok; }
private:
    QStandardItem *formItem(const QString &name) const;
private:
    bool ok;
    const CppReader &reader;
};

#endif // CPPENUMESPACE_H
