/** @file qrcfile.h
* @brief an interface for reading from files in the Qt resources.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about an interface for reading
* from files in the Qt resources which is derived
* from the QFile Class.
*/

#ifndef QRCFILE_H
#define QRCFILE_H

#include <QFile>

class QRCFile : public QFile
{
    Q_OBJECT
public:
    explicit QRCFile(const QString & name);
    
    bool open(OpenMode flags = QIODevice::ReadOnly);
    QString context(const QString &title, int id) const;
private:
    struct DataType
    {
        QString context;
        QString title;
        int id;
    };
private:
    QList<DataType> data;
};

#endif // QRCFILE_H
