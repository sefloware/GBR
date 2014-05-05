/** @file cppreader.cpp
* @brief the wrapper of the C++ code.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about locating the wrapper of the C++ code.
*/

#include "cppreader.h"
#include "assert.h"

CppReader::CppReader(const QString &str, bool simple)
    : from(0),text(str),posquery(text)
{
    if(simple)
        return;

    CppPosQuery::remove(text,CppPosQuery::Comment);

    //remove 'private:', 'protected:', and 'public:'.
    QRegExp rmbg("(?=\\bprivate\\s*:)|(?=\\bprotected\\s*:)|\\bclass\\b[^{]*\\{");
    QRegExp rmed("\\bpublic\\s*\\:|(?=\\}\\s*;)");
    int beg = 0,end = 0;
    while( (beg=posquery.find(rmbg,beg,CppPosQuery::Quot,false)) != -1)
    {
        beg += rmbg.matchedLength();
        end=posquery.find(rmed,beg,CppPosQuery::Quot,false);

        int count = (end==-1) ? text.length()- beg : end + rmed.matchedLength()-beg;
        text.remove(beg,count); //移除。
    }

    //remove '|CppPosQuery::Directive'
    //remove 'public:' in struct.
    int pos =  0;
    QRegExp rmpb("\\bpublic\\s*:|\\#[^\n]*\n");
    while((pos=posquery.find(rmpb,pos,CppPosQuery::Quot,false)) != -1)
    {
        text.remove(pos,rmpb.matchedLength());
    }

    //simplified
    text = text.simplified();
}

bool CppReader::next()
{
    if(from >= text.length() && from < 0)
        return false;

    //locate the sentance.
    QRegExp rxlc("[{;]");
    int loc = posquery.find(rxlc,from,CppPosQuery::QuotAndComment,false);
    if(loc == -1)
        return false;

    hd = text.mid(from,loc-from).trimmed();
    locch = text.at(loc);

    if(locch == QLatin1Char('{'))
    {
        int end = posquery.findPair(QRegExp("[{}]"),loc,CppPosQuery::QuotAndComment,false);
        if(end == -1)
            return false;

        bd = text.mid(loc+1,end-loc-1).trimmed();

        const QRegExp rx("\\}(\\s*;)?");
        int pos = posquery.find(rx,end);
        assert(pos == end);
        end += rx.matchedLength();
        from = end;
    }
    else
    {
        bd.clear();
        from = loc+1;
    }
    return true;
}
