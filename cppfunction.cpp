/** @file cppfunction.cpp
* @brief Parsing the Function objects and Operator objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the Function objects and
* Operator objects, which is the subroutine of the C++
* autocompleter and syntax highlighter.
*/

#include "cppfunction.h"
#include "cppvariable.h"
#include "cppreader.h"
#include "cppkeywords.h"
#include "cppobjlist.h"

QVariant FunctionItem::data(int role) const
{
    switch (role) {
    case Qt::ToolTipRole:
    {
        QStringList tips;
        foreach (QStandardItem *it, subitems) {
            tips << it->data(Qt::ToolTipRole).toString();
        }
        return tips.join('\n');
    }
    case Item::DescriptionRole:
        return description();
    default:
        return QStandardItem::data(role);
    }
}

FunctionItem::~FunctionItem()
{
    qDeleteAll(subitems);
    subitems.clear();
}

QStringList FunctionItem::description() const
{
    QStringList overlist;
    for (int i=0; i< subitems.count(); ++i)
        overlist << subitems[i]->QStandardItem::data(Item::DescriptionRole).toStringList();

    return overlist;
}

FunctionItem *FunctionItem::clone() const
{
    FunctionItem *result = new FunctionItem;
    *result = *this;
    foreach (QStandardItem *it, subitems) {
        result->subitems << it->clone();
    }

    return result;
}

QString FunctionItem::typeOf(const QString &str) const
{
    QString rdbk = str;
    //recognization?
    if( ! rdbk.startsWith('(') || ! rdbk.endsWith(')') )
        return QString();
    //yes!
    rdbk.remove(0,1).chop(1);
    QMultiMap<int,QStandardItem *> map = matchArgument( VarAttri::splitArguments(rdbk), true);
    return map.isEmpty() ? QString() : map.begin().value()->data(Item::PurifiedTypeRole).toString();
}

QMultiMap<int,QStandardItem *> FunctionItem::matchArgument(const QStringList &arg, bool exactly) const
{
    QMultiMap<int,QStandardItem *> map;
    foreach (QStandardItem *it, subitems)
    {
        QStringList arg0 = it->data(Item::NameRole).toStringList();

        if( (exactly && arg0.count() != arg.count() ) ||
                ( ! exactly && arg0.count() < arg.count() ) )
            continue;

        int degree = 0;
        for (int i=0; i< arg.count(); ++i)
            if (arg0.at(i) != arg.at(i))
                ++degree;
        map.insert(degree,it);
    }
    return map;
}

//---------
PathFunction::PathFunction(const CppReader &reader, CppObjList &objs) :
    ok(false),reader(reader)
{
    //recognization?
    QRegExp rx("\\b([a-zA-Z_]\\w*)\\s*\\(");
    //recognization?
    int pos = reader.header().indexOf(rx);
    QRegExp rxconst("\\)\\s*(const\\s*)?$");
    if(pos == -1 || ! reader.header().contains(rxconst))
        return;

    QRegExp rxtem("^template\\s+\\<");
    int offset = 0;
    if(reader.header().contains(rxtem))
    {
        CppPosQuery query(reader.header());
        offset = rxtem.matchedLength()-1;
        if( (offset = query.findPair(QRegExp("[<>]"),offset)) == -1)
                return;
        ++offset;
    }

    name = rx.cap(1);
    if(name == QLatin1String("operator") )
        return;
    type = VarAttri::purifyType(reader.header().mid(offset,pos-offset));
    if( type.isEmpty() )
        return;

    QString agstr = reader.header().mid( pos + rx.matchedLength());
    agstr.chop(rxconst.matchedLength());

    QStringList args = VarAttri::splitArguments(agstr);
    foreach (const QString &it, args)
    {
        VarAttri var(it);
        if( !var)
            return;
        argtypes << var.typeName();
    }

    ok = true;
     //yes!

    QStandardItem *item = objs.indexOf(name);
    if ( !item)
        //function doesn't exist;
        objs.append( item = formNameItem());
    else if( item->data(Item::CppTypeRole).toInt() != Item::Function)
        //the name exist but not a function.
        return;

    if( ! CppObjList::findSubitem( argtypes, static_cast<FunctionItem *> (item)->subitems))
        //the overload version doesn't exist.
        static_cast<FunctionItem *> (item)->subitems << formOverloadItem();
}

FunctionItem *PathFunction::formNameItem() const
{
    FunctionItem *item = new FunctionItem;
    item->setData(name,Item::NameRole);
    item->setForeground(Qt::blue);
    QFont _font = item->font();
    _font.setItalic(true);
    item->setFont(_font);
    item->setEditable(false);
    return item;
}

QStandardItem *PathFunction::formOverloadItem() const
{
    QStandardItem *item = new Item::CppItem(Item::Variable);
    item->setData(argtypes,Item::NameRole);
    item->setData(type,Item::PurifiedTypeRole);
    item->setData(reader.header(),Qt::ToolTipRole);
    item->setEditable(false);
    return item;
}

//-------------
PathOperator::PathOperator(CppStructReader &reader) :
    ok(false),reader(reader)
{
    //recognization? operator () or [].
    QRegExp rx("operator\\s*(\\(\\)|\\[\\])\\s*\\(");
    //recognization?
    int pos = reader.header().indexOf(rx);
    QRegExp rxconst("\\)\\s*(const\\s*)?$");
    if(pos == -1 || ! reader.header().contains(rxconst))
        return;

    QRegExp rxtem("^template\\s+\\<");
    int offset = 0;
    if(reader.header().contains(rxtem))
    {
        CppPosQuery query(reader.header());
        offset = rxtem.matchedLength()-1;
        if( (offset = query.findPair(QRegExp("[<>]"),offset,CppPosQuery::QuotAndComment)) == -1)
                return;
        ++offset;
    }

    type = VarAttri::purifyType(reader.header().mid(offset,pos-offset));
    if(type.isEmpty())
        return ;

    bool isquare = rx.cap(1) == QLatin1String("[]");

    QString agstr = reader.header().mid( pos + rx.matchedLength());
    agstr.chop(rxconst.matchedLength());
    QStringList args = VarAttri::splitArguments(agstr);
    foreach (const QString &it, args)
    {
        VarAttri var(it);
        if(!var) return;
        argtypes << var.typeName();
    }

    if(isquare && argtypes.count() != 1)
        return;

    //yes!
    ok = true;

    if( ! CppObjList::findSubitem(argtypes, (isquare ? Item::SquareOperator : Item::RoundOperator),reader.subitems))
        //the overload version doesn't exist.
        isquare ? reader.subitems << formOpitem<Item::SquareOperator>() :  reader.subitems << formOpitem<Item::RoundOperator>();
}

template <Item::CppTypeFlags flag>
QStandardItem *PathOperator::formOpitem() const
{
    QStandardItem *item = new Item::CppItem(flag);
    item->setData(argtypes,Item::NameRole);
    item->setData(type,Item::PurifiedTypeRole);
    item->setData(reader.header(),Qt::ToolTipRole);
    item->setEditable(false);
    return item;
}
