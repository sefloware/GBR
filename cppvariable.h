/** @file cppvariable.h
* @brief Parsing the Variable object.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the Variable object, which
* is the subroutine of the C++ autocompleter and
* syntax highlighter.
*/

#ifndef CPPVARIABLE_H
#define CPPVARIABLE_H
#include "item.h"

class CppReader;
class CppObjList;

class VarAttri
{
public:
    VarAttri() : ok(false) {}
    VarAttri(const QString &header);
    VarAttri(const QString &type,const QString &name,const QString &prototype);

    const QString &varName() const { return varnm; }
    const QString &typeName() const { return type; }
    const QString &protoType() const {return prototype; }

    static QString purifyType(QString type);
    static QStringList splitArguments(const QString &argstr);

    operator bool() const { return ok; }
private:
    bool ok;

    QString type;
    QString varnm;
    QString prototype;
};

//***class PathVariable***
class PathVariable
{
public:
    PathVariable(const CppReader &reader,CppObjList &objs);
    operator bool() const {return ok; }
private:
    QStandardItem *formItem(const VarAttri &var) const;
private:
    bool ok;
};

#endif // CPPVARIABLE_H
