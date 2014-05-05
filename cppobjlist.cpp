/** @file cppobjlist.cpp
* @brief Parsing the C++ objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the C++ objects, which
* is the routine of the C++ autocompleter and
* syntax highlighter.
*/

#include "cppobjlist.h"
#include "cppreader.h"
#include "cppclass.h"
#include "cppvariable.h"
#include "cppenumespace.h"
#include "cppfunction.h"
#include <QTextStream>

CppObjList &CppObjList::operator<<(CppReader &cr)
{
    while (cr.next())
    {
        switch (cr.locchar().toLatin1()) {
        case '{':
            PathClass(cr,*this) ||
                    PathEnum(cr,*this) ||
                    PathNamespace(cr,*this) ||
                    PathFunction(cr,*this);
            break;
        case ';':
            PathFunction(cr,*this) ||
                    PathVariable(cr,*this);
            break;
        }
    }
    return *this;
}

CppObjList &CppObjList::operator<<(CppStructReader &csr)
{
    while (csr.next())
    {
        switch (csr.locchar().toLatin1()) {
        case '{':
            PathClass(csr,*this) || //try to make the reader pass the class path.
                    PathEnum(csr,*this) ||
                    PathNamespace(csr,*this)||
                    PathOperator(csr) ||
                    PathFunction(csr,*this);
            break;
        case ';':
            PathOperator(csr) ||
                    PathFunction(csr,*this) ||
                    PathVariable(csr,*this);
            break;
        }
    }
    return *this;
}

QStandardItem *CppObjList::findSubitem(const QStringList &args,const QList<QStandardItem *> &subitems)
{
    foreach (QStandardItem *it, subitems)
        if( it->data(Item::NameRole).toStringList() == args)
            return it;
    return 0;
}

QStandardItem *CppObjList::findSubitem(const QStringList &args, int flag, const QList<QStandardItem *> &subitems)
{
    foreach (QStandardItem *it, subitems)
        if( it->data(Item::NameRole).toStringList() == args && it->data(Item::CppTypeRole).toInt() == flag)
            return it;
    return 0;
}

QStandardItem *CppObjList::indexOf(const QString &text) const
{
    assert(!text.isEmpty());

    foreach (QStandardItem *it, *this)
        if(it->data(Item::NameRole).toString() == text)
            return it;
    return 0;
}

void CppObjList::readDescription(const QString &text)
{
    QTextStream in((QString *)(&text),QIODevice::ReadOnly);
    readDescription(*this,in);
}

void CppObjList::readDescription(const QList<QStandardItem *> &itemList,QTextStream &in)
{
    if(in.atEnd()) return;

    QMap<QStandardItem *,QList<QStandardItem *> > map;
    QList<QStandardItem *> item_list;

    foreach (QStandardItem *it, itemList) {
        assert(it);

        switch(it->data(Item::CppTypeRole).toInt())
        {
        case Item::Function:
            item_list << static_cast<FunctionItem *>(it)->subitems;
            break;
        case Item::Class:
        {
            item_list << it;

            QList<QStandardItem *> oplist = static_cast<ClassItem *>(it)->subitems;
            for (int j=0; j<it->rowCount(); ++j)
                oplist << it->child(j);

            if(! oplist.isEmpty())
                map.insert(it,oplist);
            break;
        }
        case Item::Namespace:
        {
            item_list << it;

            QList<QStandardItem *> oblist;
            for (int j=0; j<it->rowCount(); ++j)
                oblist << it->child(j);

            if(! oblist.isEmpty())
                map.insert(it,oblist);

            break;
        }
        case Item::Variable:
        case Item::SquareOperator:
        case Item::RoundOperator:
           item_list << it;
           break;
        }
    }

    QStandardItem *item = 0;

    while ( (item || item_list.count()) && ! in.atEnd())
    {
        QString lineText = in.readLine().trimmed();

        if(lineText.isEmpty())
            continue;

        if(! lineText.startsWith("//") )
        {
            lineText = lineText.simplified();
            for (int i=0; i<item_list.count(); ++i)
            {
                const QString proto = item_list[i]->QStandardItem::data(Qt::ToolTipRole).toString();
                if( lineText.startsWith(proto))
                {
                    item = item_list.takeAt(i);

                    lineText = lineText.mid(proto.length());
                    lineText.remove(QRegExp("^\\s*(?:;\\s*)?"));
                    break;
                }

            }
        }

        if(! item || lineText.isEmpty())
            continue;

        if(lineText.startsWith("//"))
        {
            QStringList des = item->QStandardItem::data( Item::DescriptionRole).toStringList();
            des << lineText.mid(2);
            item->setData(des, Item::DescriptionRole);
        }
        else
        {
            if(map.contains(item))
                readDescription(map.value(item),in);
            item = 0;
        }
    }
}
