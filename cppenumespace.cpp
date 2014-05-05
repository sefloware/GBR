/** @file cppenumespace.cpp
* @brief Parsing the Enum objects and the Namespace objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the Enum objects and the
* Namespace objects, which is the subroutine of the
* C++ autocompleter and syntax highlighter.
*/

#include "cppenumespace.h"
#include "cppreader.h"
#include "cppobjlist.h"

//*** class PathEnum ***
PathEnum::PathEnum(const CppReader &reader, CppObjList &objs):
    ok(false), reader(reader)
{
    //recognization?
    if( !reader.header().startsWith("enum"))
        return;
    ok = true;
    //yes!

    //whether exists the enum name?
    QString name;
    QRegExp rn("^enum\\s+([a-zA-Z_]\\w*)\\s*$");
    if(reader.header().contains(rn) &&
            ! objs.indexOf( name = rn.cap(1)) )        //query for the existance of the name.
    {
        QStandardItem *item = new Item::CppItem(Item::Enum);
        item->setForeground(Qt::darkMagenta);
        item->setData(rn.cap(1),Item::NameRole);
        item->setData(reader.header(),Qt::ToolTipRole);
        item->setEditable(false);
        objs.append( item);
    }

    //the content of the enum.
    QStringList list = reader.body().split(',',QString::SkipEmptyParts);
    QRegExp rx("^\\s*([a-zA-Z_]\\w*)\\s*(?:=[^,;]+)?$");
    foreach (const QString &it, list)
    {
        if(! it.contains(rx) )
            continue;
        if( ! objs.indexOf( rx.cap(1)))
        {
            QStandardItem *item = new Item::CppItem(Item::Enumerator);
            item->setForeground(Qt::darkMagenta);
            item->setData(rx.cap(1),Item::NameRole);
            item->setData("enum: " + it,Qt::ToolTipRole);
            item->setData( name.isEmpty() ? "int" : name, Item::PurifiedTypeRole);
            item->setEditable(false);
            objs.append( item);
        }
    }
}

//*** class PathNamespace ***
PathNamespace::PathNamespace(const CppReader &reader, CppObjList &objs) :
    ok(false), reader(reader)
{
    //recognization?
    QRegExp rn("^namespace\\s+([a-zA-Z_]\\w*)\\s*$");
    if(! reader.header().contains(rn))
        return ;
    ok = true;
    //yes!

    //the NamespaceItem.
    QString name = rn.cap(1);
    QStandardItem *item = objs.indexOf(name);
    if( item == 0)
        objs.append(item = formItem(name) );

    //the content of the namespace.
    CppObjList childrens;
    CppReader sr(reader.body());
    childrens << sr;
    foreach (QStandardItem *it, childrens) {
        item->appendRow(it);
    }
}

QStandardItem *PathNamespace::formItem(const QString &name) const
{
    assert(!name.isEmpty());

    QStandardItem *item = new Item::CppItem(Item::Namespace);
    item->setData(name,Item::NameRole);
    item->setData(reader.header(),Qt::ToolTipRole);
    item->setForeground(Qt::darkMagenta);
    item->setEditable(false);
    return item;
}


