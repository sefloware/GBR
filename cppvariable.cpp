/** @file cppvariable.cpp
* @brief Parsing the Variable object.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the Variable object, which
* is the subroutine of the C++ autocompleter and
* syntax highlighter.
*/

#include "cppvariable.h"
#include "cppposquery.h"
#include "cppobjlist.h"
#include "cppkeywords.h"
#include "editorobjmodel.h"
#include "cppreader.h"
#include "config.h"

//***struct VarAttri***
VarAttri::VarAttri(const QString &header) : ok(false)
{
    prototype = header.simplified();

    QRegExp rx("\\b([a-zA-Z_]\\w*) ?([=(]|(?:\\[ ?\\w* ?\\] ?)+|$)");
    int pos = header.indexOf(rx);
    if(pos == -1)
        return;

    type = purifyType(header.left(pos));
    if(type.isEmpty())
        return;

    int n = rx.cap(2).count('[');
    type.append( QString(n,'*'));
    varnm = rx.cap(1);
    ok = true;
}

VarAttri::VarAttri(const QString &type, const QString &name, const QString &prototype) :
    ok(true),type(type),varnm(name),prototype(prototype) { }

QString VarAttri::purifyType(QString type)
{
    foreach (const QString &it, cppkeys().others)
        type.remove(QRegExp("\\b"+it+"\\b") );

    type.remove(QRegExp("\\s+")).remove(QRegExp("<[^>]*>")).remove('&').replace("[]","*");
    QRegExp rx("^[a-zA-Z_]\\w*(?:::[a-zA-Z_]\\w*)*\\**$");
    if( !type.contains(rx))
        return QString();
    return type;
}

QStringList VarAttri::splitArguments(const QString &argstr)
{
    //skip ().
    QStringList result;

    int pos = 0;
    int anchor = 0;

    while( ( pos = argstr.indexOf(',', pos) ) != -1)
    {
        if(argstr.leftRef(pos).count('(') == argstr.leftRef(pos).count(')') &&
              argstr.leftRef(pos).count('<') == argstr.leftRef(pos).count('>')   )
        {
            result << argstr.mid(anchor,pos-anchor).trimmed();
            anchor = pos+1;
        }
        ++pos;
    }

    QString laststr = argstr.mid(anchor).trimmed();
    if(laststr.isEmpty() && result.isEmpty() )
        return result;
    result << laststr;//the last one.
    return result;
}

//***class PathVariable***
PathVariable::PathVariable(const CppReader &reader, CppObjList &objs) :
   ok(false)
{
    QString header = reader.header().simplified();
    if(header.isEmpty())
        return;

    QStringList list = VarAttri::splitArguments(header );
    //first var
    assert( !list.isEmpty());

    QString str1st = list.takeAt(0);
    VarAttri var1st(str1st);
    if(!var1st || objs.indexOf(var1st.varName()))
        return;
    objs.append(formItem(var1st) );

    //prototype prefix.
    int pos = var1st.protoType().indexOf(var1st.varName());
    assert(pos != -1);
    QString profix = var1st.protoType().left(pos);
    profix.remove('&').remove('*');

    //left var
    foreach (const QString &it, list) {
        QString str = it.simplified();
        QRegExp rx("^([*& ]*)\\b([a-zA-Z_]\\w*) ?(=.+|\\([^)]*\\)|(?:\\[[^]]*\\] ?)+)?$");
        if( !str.contains(rx) )
            break;
        QString type = var1st.typeName();
        type.remove('*');

        int n = rx.cap(1).count('*');
        if(rx.cap(3).startsWith('['))
            n += rx.cap(3).count('[');
        type.append( QString(n,'*'));

        str.prepend(' ').prepend(profix);
        VarAttri var(type,rx.cap(2),str.simplified());
        if(objs.indexOf(var.varName()))
            break;
        objs.append( formItem(var) );
    }

    ok = true;
}

QStandardItem *PathVariable::formItem(const VarAttri &var) const
{
    assert(var);
    Item::CppItem *item = new Item::CppItem(Item::Variable);
    item->setData(var.varName(),Qt::EditRole);
    item->setData(var.typeName(),Qt::WhatsThisRole);
    item->setData(var.protoType(),Qt::ToolTipRole);
    item->setForeground(Qt::darkRed);
    item->setEditable(false);
    return item;
}
