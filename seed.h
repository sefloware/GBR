/** @file seed.h
* @brief the random seeds.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about getting the random seeds from the
* seed cache or the WWW.
*/

#ifndef SEED_H
#define SEED_H

#include <QDir>

QT_BEGIN_NAMESPACE
class QString;
class QByteArray;
QT_END_NAMESPACE

class seedsource
{
public:
    seedsource(const QString &path);
    bool operator() (int count);
    static bool cache();
private:
    static QByteArray getfromweb(int count = 10000);
    static QString getfromcache (int count);
private:
    QDir _dir;
};

#endif // SEED_H
