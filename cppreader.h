/** @file cppreader.h
* @brief the wrapper of the C++ code.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about locating the wrapper of the C++ code.
*/

#ifndef CPPREADER_H
#define CPPREADER_H

#include "cppposquery.h"

QT_BEGIN_NAMESPACE
class QStandardItem;
QT_END_NAMESPACE

class CppReader
{
public:
    //when the simple is true, it means the str is in briefness.
    explicit CppReader (const QString &str,bool simple = false);
    bool next();
    const QString &header() const { return hd; }
    const QString &body() const { return bd; }
    const QChar &locchar() const { return locch;}
private:
    int from;
    QString text;
    CppPosQuery posquery;

    QString hd;
    QString bd;
    QChar locch;
};

class CppStructReader : public CppReader
{
public:
    explicit CppStructReader(const QString &str,QList<QStandardItem *> &subitems)
        : CppReader(str,true),subitems(subitems) {}
    QList<QStandardItem *> &subitems;
};

#endif // CPPREADER_H
