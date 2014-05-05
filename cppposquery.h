/** @file cppposquery.h
* @brief locating the C++ objects in parsing the C++ objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about locating the C++ objects in parsing the C++
* objects.
*/

#ifndef CPPPOSQUERY_H
#define CPPPOSQUERY_H

#include <QPair>
#include <QRegExp>
#include <QMap>

class CppPosQuery
{
public:
    enum Flags{NoFlag=0,
               DoubleQuot=1,
               SingleQuot=2,
               SingleLineComment=4,
               SingleExclamLineComment=8,
               MultiLineComment=16,
               Other=64,

               Quot = DoubleQuot|SingleQuot,
               Comment = SingleLineComment|SingleExclamLineComment|MultiLineComment,
               QuotAndComment = Quot|Comment
              };
    explicit CppPosQuery(const QString &text);

    int flag(int position) const;
    const QString &text() const
    { return _text; }
    static QString &remove(QString &_text, int removeFlag);
public:
    bool isEscaped(int pos) const;
    int find(const QChar &to, int from, int skipFlags = NoFlag, bool skipEscchar = false) const;
    int find(const QRegExp &to, int from, int skipFlags = NoFlag, bool skipEscchar = false) const;
    int find(const QString &to, int from, int skipFlags = NoFlag, bool skipEscchar = false) const;

    int findPair(const QRegExp &pair, int from, int skipFlags = NoFlag, bool skipEscchar = false) const;
private:
    QRegExp rxnc; //begin of non-cpp clause;
    const QString &_text;
    QMap<QString,QPair<QString,Flags> > mapnc; //the map of the pairs of nonterminor charcters.
};

#endif // CPPPOSQUERY_H
