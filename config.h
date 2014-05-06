/** @file config.h
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

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <QMap>
#include "assert.h"

class cppkeywords;
QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QString;
class QStringList;
class QWidget;
QT_END_NAMESPACE

enum MyInfoFlag {HomePath, DebugCMD};
QStringList &myInfos();

cppkeywords &cppkeys();
bool insureDirectory(const QString &directory);

enum MyNameFlags {ProjectFolder, SourceFolder, ConfigFolder,InfoFile};
QStringList &readInfo();
bool writeInfo(const QStringList &infos);

extern const QString MyNames[];
QString absolutePath(MyNameFlags flag);

QMap<QString, QString> parseIdentification(const QString &identification);
QString identification(const QString &name, const QStringList &outputs, const QStringList &ids, const QString version);

#endif // CONFIG_H
