/** @file qrcfile.cpp
* @brief an interface for reading from files in the Qt resources.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about an interface for reading
* from files in the Qt resources which is derived
* from the QFile Class.
*/

#include "qrcfile.h"
#include <QRegExp>

QRCFile::QRCFile(const QString &name) :
    QFile(name)
{
}
bool QRCFile::open(OpenMode flags)
{
    if(!QFile::open(flags)) return false;
    QString text = this->readAll();
    QRegExp rxbeg("//! title (\\w+) (\\d+)");
    QString strend;
    int beg=0,end=0;
    while( ((beg=text.indexOf(rxbeg,beg))!=-1) &&
           (end=text.indexOf( (strend=rxbeg.cap().append(" end")),beg))!=-1)
    {
        DataType record;
        beg += rxbeg.matchedLength();
        record.context = text.mid(beg,end-beg).trimmed();
        record.title = rxbeg.cap(1);
        record.id = rxbeg.cap(2).toInt();
        beg = end+strend.length();
        data << record;
    }
    return true;
}

QString QRCFile::context(const QString &title,int id) const
{
    foreach (const DataType &it, data)
        if(it.title == title && it.id == id)
            return it.context;
    return QString();
}
