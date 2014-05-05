/** @file editormap.cpp
* @brief the C++ object model map.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the C++ object model map Class.
*/

#include "editormap.h"
#include "item.h"
#include "assert.h"

CppObjectModelMap::CppObjectModelMap()
{
    include = includeItem();
}

CppObjectModelMap::~CppObjectModelMap()
{
    QMapIterator<QString, QStandardItemModel *> i(*this);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
    delete include;
}

QMap<QString,QStandardItemModel *>::iterator CppObjectModelMap::insert(const QString &name, QStandardItemModel *model)
{
    if(name.isEmpty() || model == 0)
        return QMap<QString,QStandardItemModel *>::iterator();

    if(contains(name))
        delete static_cast<QStandardItemModel *>(take(name));
    else
        include->appendRow(fileItem(name));
    return QMap::insert(name,model);
}

QStandardItem *CppObjectModelMap::includeItem()
{
    QStandardItem *result = new Item::CppItem(Item::Include);
    result->setData("#include",Qt::EditRole);
    result->setData("include directive",Qt::ToolTipRole);
    result->setForeground(Qt::darkGreen);
    result->setEditable(false);
    return result;
}

QStandardItem *CppObjectModelMap::fileItem(const QString &fileName)
{
    assert(!fileName.isEmpty());

    QStandardItem *fileItem = new Item::CppItem(Item::HFile);
    fileItem->setData(fileName,Qt::EditRole);
    fileItem->setData("file name",Qt::ToolTipRole);
    fileItem->setForeground(Qt::darkGreen);
    fileItem->setEditable(false);
    return fileItem;
}
