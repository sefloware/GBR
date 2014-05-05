/** @file cppposquery.cpp
* @brief locating the C++ objects in parsing the C++ objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about locating the C++ objects in parsing the C++
* objects.
*/

#include "cppposquery.h"
#include <QRegExp>
#include "assert.h"

CppPosQuery::CppPosQuery(const QString &text) :
    rxnc("([\'\"]|//\\!|//|/\\*)"),_text(text)
{
    mapnc.insert("\'",qMakePair(QString("\'"),SingleQuot));
    mapnc.insert("\"",qMakePair(QString("\""),DoubleQuot));
    mapnc.insert("//",qMakePair(QString("\n"),SingleLineComment));
    mapnc.insert("//!",qMakePair(QString("\n"),SingleExclamLineComment));
    mapnc.insert("/*",qMakePair(QString("*/"),MultiLineComment));
}

QString &CppPosQuery::remove(QString &text, int removeFlag)
{
    CppPosQuery posquery(text);
    //remove CppPosQuery::Comment|CppPosQuery::Directive
    for(int i=0;i<text.length();)
    {
        if(posquery.flag(i) & removeFlag )
        {
            int pos = i;
            while(posquery.flag(++pos) & removeFlag);
            text.remove(i,pos-i);
        }
        else
            ++i;
    }
    return text;
}

bool CppPosQuery::isEscaped(int pos) const
{
    assert( pos>-1 && pos<_text.size() );

    if((--pos) < 0) return false; //to the end
    int scount =0; //the count of backslash
    while(_text.at(pos) == QLatin1Char('\\'))
    {
        ++scount;
        if( (--pos) < 0) break;
    }
    return scount%2 == 1;
}
int CppPosQuery::find(const QChar &to, int from, int skipFlags, bool skipEscchar) const
{
    while ( ((from = _text.indexOf(to,from)) != -1) &&
            ( (skipEscchar && isEscaped(from)) || (flag(from) & skipFlags) )    )
    {
        ++from;
    }
    return from;
}

int CppPosQuery::find(const QRegExp &to, int from, int skipFlags, bool skipEscchar) const
{
    while ( ((from = to.indexIn(_text,from)) != -1) &&
            ( (skipEscchar && isEscaped(from)) || (flag(from) & skipFlags) )    )
    {
        from += to.matchedLength();
    }
    return from;
}

int CppPosQuery::find(const QString &to, int from, int skipFlags, bool skipEscchar) const
{
    while ( ((from = _text.indexOf(to,from)) != -1) &&
            ( (skipEscchar && isEscaped(from)) || (flag(from) & skipFlags) )    )
    {
        from += to.length();
    }
    return from;
}

int CppPosQuery::findPair(const QRegExp &pair, int from, int skipFlags, bool skipEscchar) const
{
    if(from < 0 || from >= _text.size())
        return -1;
    assert(find(pair,from,skipFlags,skipEscchar) == from );

    const QChar ch = _text.at(from);

    int count = 1;
    while (count != 0)
    {
        ++from;
        from = find(pair,from,skipFlags,skipEscchar);
        if(from == -1)
            return -1;

        if(_text.at(from) == ch)
            ++count;
        else
            --count;
    }
    return from;
}

int CppPosQuery::flag(int position) const
{
    if(position < 0 || position >= _text.size())
        return NoFlag;

    int pos = 0;
    while( (pos=rxnc.indexIn(_text,pos)) != -1 &&
           pos <= position ) //include the boundary characters.
    {
        pos += rxnc.matchedLength();
        QString key = rxnc.cap();
        assert(mapnc.contains(key));

        QPair<QString,Flags> pr = mapnc.value(key);
        if(pr.second == MultiLineComment)
        {
            pos = _text.indexOf( pr.first, pos);
            if(pos == -1)
                return pr.second;
            pos += pr.first.length();
        }
        else do
        {
            pos = _text.indexOf( pr.first, pos);
            if(pos == -1)
                return pr.second;
            pos += pr.first.length();
        }
        while(pos < _text.size() && isEscaped(pos) );

        if(pos > position) //include boundary symbols.
            return pr.second;
    }
    return Other;
}
