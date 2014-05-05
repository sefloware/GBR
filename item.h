/** @file item.h
* @brief the base cpp item Class
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file define the CppItem Class, the base C++
* item and some item-data roles.
*/

#ifndef ITEM_H
#define ITEM_H
#include <QStandardItem>
#include "assert.h"

namespace Item
{
enum CppItemRoles{ PurifiedTypeRole = Qt::WhatsThisRole,
                   NameRole = Qt::EditRole,
                   CppTypeRole = Qt::UserRole+1,
                   ParentIndexRole = Qt::UserRole+2,
                   DescriptionRole = Qt::UserRole+5
                 };

enum CppTypeFlags{ Class=0,
                   Namespace,
                   Enum,
                   Enumerator,
                   Function,
                   Variable,
                   SquareOperator,
                   RoundOperator,
                   Include,
                   TypeKeyword,
                   NonTypeKeyword,
                   HFile
                 };

enum HeaderItemRoles{ HeaderFlagRole = Qt::UserRole+3,
                      DataRole = Qt::UserRole+4
                    };
enum HeaderFlags{ NoFlag=0,
                  Id=1,
                  Parameter=2,
                  Parafiter=4,
                  Times=8,
                  FileSt=16,
                  FileOt=32,
                  Formula=64
                };

enum VerticalItemFlags{ OtherRow = 0,
                        SimData = 1,
                        NoSimData = 2
                      };

const QMap<CppTypeFlags,QIcon> &cppIconMap();
const QMap<HeaderFlags,QIcon> &headerIconMap();

class CppItem : public QStandardItem
{
public:
    explicit CppItem(CppTypeFlags T)
    {
        QStandardItem::setData(T,CppTypeRole);
        QStandardItem::setIcon(cppIconMap().value(T,QIcon()));
    }

    explicit CppItem(CppTypeFlags T,const QString & text)
        : QStandardItem(text)
    {
        this->setData(T,CppTypeRole);
        this->setIcon(cppIconMap().value(T,QIcon()) );
    }
};

}

#endif // ITEM_H
