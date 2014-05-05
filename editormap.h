/** @file editormap.h
* @brief the C++ object model map.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the C++ object model map Class.
*/

#ifndef EDITORMAP_H
#define EDITORMAP_H

#include <QMap>

QT_BEGIN_NAMESPACE
class QStandardItem;
class QStandardItemModel;
QT_END_NAMESPACE

class CppObjectModelMap : public QMap<QString,QStandardItemModel *>
{  
public:
    CppObjectModelMap();
    ~CppObjectModelMap();
    iterator insert(const QString &name, QStandardItemModel *model);
private:
    QStandardItem *includeItem();
    QStandardItem *fileItem(const QString &fileName);
public:
    QStandardItem *include;
};

#endif // EDITORMAP_H
