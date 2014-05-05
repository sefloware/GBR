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
#include "compenv.h"
#include <QStandardItemModel>
#include <QString>
#include <QDir>
#include <QFile>

cppkeywords &cppkeys()
{
    static cppkeywords model;
    return model;
}

QStandardItemModel &runevmodel()
{
    static QStandardItemModel model;
    return model;
}

const QString folderName[] = {"project", "source", "configure" };

QString absolutePath(FolderFlag flag)
{
    if(! insureDirectory(folderName[flag]))
        return QString();

    return QDir(folderName[flag]).absolutePath();
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

void editrunenv(QWidget *parent)
{
    QString path = absolutePath(ConfigFolder);
    if( path.isEmpty())
        return;

    path = QDir(path).absoluteFilePath("runenv");
    QFile file(path);
    if (! file.open(QIODevice::ReadWrite))
        return;

    CEDialog dialog(parent);
    dialog.setPlainText( file.readAll());
    dialog.setWindowFilePath(path);
    file.close();

    if( dialog.exec() && dialog.isWindowModified())
    {
        if(! file.open(QIODevice::WriteOnly | QIODevice::Truncate) )
            return;

        QString text = dialog.plainText();
        file.write(text.toLatin1());
        file.close();
        readRunEnv();
    }
}

void readRunEnv()
{
    const QString path = absolutePath(ConfigFolder);
    if(path.isEmpty())
        return;

    QFile file(QDir(path). absoluteFilePath("runenv"));
    if (! file.open(QIODevice::ReadOnly))
        return ;

    runevmodel().setRowCount(0);

    QByteArray data = file.readAll();
    std::string line = data.data();

    using boost::spirit::ascii::space;
    std::string::iterator iter = line.begin();
    std::string::iterator end = line.end();
    std::vector<client::runenvtype> result;
    client::RunenvGrammar<std::string::iterator> rg;
    phrase_parse(iter, end, rg, space, result) ;

    for (unsigned i=0; i<result.size(); ++i)
    {
        client::runenvtype &it = result[i];
        std::string &platform = it.platform;
        std::transform(platform.begin(), platform.end(), platform.begin(), ::tolower);
        std::map<std::string,CE::PlatForm>::const_iterator flag = CE::paltformmap().find(platform);
        if( flag == CE::paltformmap().end())
            continue;
        QStandardItem *item = new QStandardItem(QString::fromStdString(it.show()).trimmed() );
        item->setData(QString::fromStdString(it.threadnum).trimmed(),CE::ThreadNumRole);
        item->setData(flag->second, CE::PlatformRole);
        item->setData(QString::fromStdString(it.compiler.name).trimmed(), CE::CNameRole);
        item->setData(QString::fromStdString(it.compiler.optimize).trimmed(),CE::COptimizationRole);
        item->setData(QString::fromStdString(it.compiler.output).trimmed(),CE::COutputRole);
        item->setData(QString::fromStdString(it.compiler.include).trimmed(),CE::CIncludepathRole);
        item->setData(QString::fromStdString(it.compiler.macro).trimmed(),CE::CMacroRole);
        item->setData(QString::fromStdString(it.toottip()).trimmed(),Qt::ToolTipRole);
        runevmodel().appendRow(item);
    }
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
