/** @file config.cpp
* @brief Reading configuration files
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file provides methods for reading configuration files,
* such as computing profiles, project logo files. It also
* provides some function to return the path of some certain
* folders in one project, and one model that contains the C++
* key words.
*/

#include "config.h"
#include "cppkeywords.h"
#include <QStandardItemModel>
#include <QString>
#include <QDir>
#include <QFile>
#include <QDebug>

QStringList &myInfos()
{
    static QStringList info;
    return info;
}

cppkeywords &cppkeys()
{
    static cppkeywords model;
    return model;
}

const QString MyNames[] = {"project", "source", "script.run" , "setting"};

QStringList &readInfo()
{
    QStringList &infos = myInfos();
    infos.clear();

    QString filepath = absolutePath(InfoFile);
    if(filepath.isEmpty())
        return infos;

    QFile file(filepath);
    if(! file.open(QIODevice::ReadOnly) )
        return infos;
    QTextStream in(&file);
    while(! in.atEnd())
        infos << in.readLine();

    return infos;
}

bool writeInfo(const QStringList &infos)
{
    QFile file(absolutePath(InfoFile));
    if(! file.open(QIODevice::WriteOnly) )
        return false;
    QTextStream out(&file);
    foreach (const QString &it, infos) {
        out << it;
    }
    return true;
}

QString absolutePath(MyNameFlags flag)
{
    switch (flag)
    {
    case InfoFile:
    {
        QFileInfo info(MyNames[flag]);

        if(info.exists() && ! info.isFile())
            return QString();
        else
            return info.absoluteFilePath();
        break;
    }
    default:
    {
        if( ! insureDirectory(MyNames[flag]))
            return QString();
        else
            return QDir(MyNames[flag]).absolutePath();
        break;
    }
    }
}


bool insureDirectory(const QString &directory)
{
    // make dir if not exists.
    //return true if path is a dir, else false.
    QDir dir(directory);
    if(! dir.exists())
        return QDir().mkpath(directory);

    if (! QFileInfo(directory).isDir() )
        return false;

    return true;
}

QString identification(const QString &name, const QStringList &outputs, const QStringList &ids, const QString version)
{
    QStringList list;
    list << "Project   : " + name;
    list << "Time_stamp: " + version ;
    list << "Output    : " + outputs.join(',');
    list << "ID        : " + ids.join(',');
    return list.join('\n');
}

QMap<QString,QString> parseIdentification(const QString &identification)
{
    QMap<QString,QString> result;

    QStringList list = identification.split('\n',QString::SkipEmptyParts);

    if(list.count() != 4)
        return QMap<QString,QString>();

    if(     ! list[0].startsWith("Project   : ") ||
            ! list[1].startsWith("Time_stamp: ") ||
            ! list[2].startsWith("Output    : ") ||
            ! list[3].startsWith("ID        : ") )
        return QMap<QString,QString>();

    result.insert("Project",    list[0].mid(12));
    result.insert("Time_stamp", list[1].mid(12));
    result.insert("Output",     list[2].mid(12));
    result.insert("ID",         list[3].mid(12));

    return result;
}
