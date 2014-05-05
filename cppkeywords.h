/** @file cppkeywords.h
* @brief Parsing the C++ keyword objects.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about parsing the C++ keyword objects,
* which is the subroutine of the C++ autocompleter and
* syntax highlighter.
*/

#ifndef CPPKEYWORDS_H
#define CPPKEYWORDS_H
#include "cppclass.h"
#include <QFile>
#include <QTextStream>
#include <QStandardItemModel>

class cppkeywords : public QStandardItemModel
{
    Q_OBJECT
public:
    cppkeywords(QObject *parent = 0)
        : QStandardItemModel(parent)
    {
        QFile file(":/file/files/cppkeys");
        if(!file.open(QIODevice::ReadOnly))
            return;
        QTextStream out(&file);

        //the first line in the file is about the type keys.
        types = out.readLine().split(QRegExp("\\s+"),QString::SkipEmptyParts);
        types.removeDuplicates();
        foreach (const QString &it, types)
        {
            QStandardItem *item = new Item::CppItem(Item::TypeKeyword);
            item->setData(it,Item::NameRole);
            item->setForeground(Qt::darkYellow);
            appendRow(item);
        }

        //the other keys.
        while(!out.atEnd())
            others << out.readLine().split(QRegExp("\\s+"),QString::SkipEmptyParts);
        others.removeDuplicates();
        foreach (const QString &it, others)
        {
            QStandardItem *item = new Item::CppItem(Item::NonTypeKeyword);
            item->setData(it,Item::NameRole);
            item->setForeground(Qt::darkYellow);
            appendRow(item);
        }
    }
public:
    QStringList types;
    QStringList others;
};

#endif // CPPKEYWORDS_H
