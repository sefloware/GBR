/** @file cppclass.cpp
* @brief Parsing the Class objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the Class objects, which
* is the subroutine of the C++ autocompleter and
* syntax highlighter.
*/

#include "cppclass.h"
#include "cppreader.h"
#include "cppvariable.h"
#include "cppobjlist.h"

QString ClassItem::typeOf(const QString &str) const
{
    if(str.startsWith('(') && str.endsWith(')') )
    {
        QString args = str;
        args.remove(0,1).chop(1);
        args = args.simplified();

        QStringList arglist;
        if( !args.isEmpty())
            arglist = VarAttri::splitArguments(args);

        QMultiMap<int,QStandardItem *> map = findRoundOperators( arglist, true);
        return map.isEmpty() ? 0 : map.begin().value()->data(Item::PurifiedTypeRole).toString();
    }
    else if(str.startsWith('[') && str.endsWith(']') )
    {
        QString arg = str;
        arg.remove(0,1).chop(1);
        QStandardItem *item = findSquareOperator(arg);
        if(! item)
            return QString();
        return item->data(Item::PurifiedTypeRole).toString();
    }
    else
        return QString();
}

ClassItem *ClassItem::clone() const
{
    ClassItem *result = new ClassItem;
    *result = *this;
    foreach (QStandardItem *it, subitems) {
        result->subitems << it->clone();
    }
    return result;
}

ClassItem::~ClassItem()
{
    qDeleteAll(subitems);
    subitems.clear();
}

QVariant ClassItem::data(int role) const
{
    switch (role) {
    case Qt::ToolTipRole:
    {
        QString tip = QStandardItem::data(Qt::ToolTipRole).toString();

        QStringList ops;
        foreach (QStandardItem *it, subitems)
            ops << it->data(Qt::ToolTipRole).toString();

        if(! ops.isEmpty())
            tip.append("\n\n").append(ops.join('\n'));

        return tip;
    }
    case Item::DescriptionRole:
        return description();
    default:
        return QStandardItem::data(role);
    }
}

QStringList ClassItem::description() const
{
    QStringList result;
    foreach (const QString &it, QStandardItem::data(Item::DescriptionRole).toStringList())
        result << it;

    for (int i=0; i< subitems.count(); ++i)
    {
        result << "--" + subitems[i]->data(Qt::ToolTipRole).toString();
        QStringList opdeslist = subitems[i]->data(Item::DescriptionRole).toStringList();
        foreach (const QString &var, opdeslist)
            result << "--" + var;
    }

    return result;
}

QStandardItem *ClassItem::findSquareOperator(const QString &arg) const
{
    if(arg.isEmpty())
        //al should not be empty.
        return 0;

    QMultiMap<int,QStandardItem *> map;
    foreach (QStandardItem *it, subitems)
    {
        if( it->data(Item::CppTypeRole).toInt() != Item::SquareOperator)
            continue;

        QString arg0 = it->data(Item::NameRole).toString();
        if(arg == arg0)
            //strictly matched.
            map.insert(0,it);
        else
            map.insert(1,it);
    }

    return map.isEmpty() ? 0 : map.begin().value(); //the first.
}

QMultiMap<int,QStandardItem *> ClassItem::findRoundOperators(const QStringList &arg, bool exactly) const
{
    QMultiMap<int,QStandardItem *> map;
    foreach (QStandardItem *it, subitems)
    {
        if( it->data(Item::CppTypeRole).toInt() != Item::RoundOperator)
            continue;

        QStringList arg0 = it->data(Item::NameRole).toStringList();

        if( (exactly && arg0.count() != arg.count() ) ||
                (! exactly && arg0.count() < arg.count() ) )
            continue;

        int degree = 0;
        for (int i=0; i< arg.count(); ++i)
            if (arg0.at(i) != arg.at(i))
                ++degree;
        map.insert(degree,it);
    }

    return map;
}

PathClass::PathClass(const CppReader &reader, CppObjList &objs) :
    ok(false), reader(reader)
{
    //the recognization?
    QRegExp rn("^(?:class|struct)\\s+([a-zA-Z_]\\w*\\b)");
    if( ! reader.header().contains(rn))
        return ;
    ok = true;
    //yes!

    QString name = rn.cap(1);
    //query for the existance of the name.
    QStandardItem *item = objs.indexOf(name);
    if (! item )
        //nonexist, so new and append a ClassItem to objs.
        objs.append(item = formItem(name) );
    else if (item->data(Item::CppTypeRole).toInt() != Item::Class)
        //exist the name but not a class.
        return;

    //the content of the class.
    CppObjList childrens;
    CppStructReader sr(reader.body(),static_cast<ClassItem *>(item)->subitems);
    childrens << sr;
    foreach (QStandardItem *it, childrens) {
        item->appendRow(it);
    }
}

ClassItem *PathClass::formItem(const QString &name) const
{
    assert(!name.isEmpty());
    ClassItem *item = new ClassItem;
    item->setData(name,Item::NameRole);
    item->setData(reader.header(),Qt::ToolTipRole);
    item->setForeground(Qt::darkMagenta);
    item->setEditable(false);
    return item;
}
