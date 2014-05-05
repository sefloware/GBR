/** @file seed.cpp
* @brief the random seeds.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about getting the random seeds from the
* seed cache or the WWW.
*/

#include "seed.h"
#include "seeddialog.h"
#include <fstream>
#include <iterator>
#include <QDir>
#include <cstdio>
#include <QTextStream>
#include "assert.h"

seedsource::seedsource(const QString &path) :
    _dir(path) { }

bool seedsource::operator() (int count)
{
    QFile file(_dir.absoluteFilePath("seeds") );
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    QFile file_mark(_dir.absoluteFilePath("mark"));
    if (!file_mark.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    QString _seeds = getfromcache(count);
    if(_seeds.isEmpty())
    {
        _seeds = getfromweb(count);
        cache();
    }

    file.write(_seeds.toLocal8Bit());
    file_mark.write("0");
    return true;
}

bool seedsource::cache()
{
    QFile _file("seeds.cache");
    if(!_file.open(QIODevice::Truncate|QIODevice::WriteOnly))
        return false;
    _file.write(getfromweb());
    return true;
}

QString seedsource::getfromcache (int count)
{
    std::ifstream _in("seeds.cache");
    if(!_in) return QString();
    std::istream_iterator<long> end;         // end-of-stream iterator
    std::istream_iterator<long> beg(_in);   // stdin iterator

    QString _result;
    QTextStream result_out(&_result);

    for(int i=0; i < count ; ++i)
    {
        if(beg == end) return QString();
        result_out << *beg << endl;
        if(_in.fail()) return QString();
        ++beg;
    }

    std::ofstream _out("temp",std::ios::trunc);
    if(_out)
    {
        _in.ignore();
        _out << _in.rdbuf();

        _in.close();
        _out.close();

        std::remove("seeds.cache");
        std::rename("temp","seeds.cache");
    }

    return _result;
}

QByteArray seedsource::getfromweb(int count)
{
    SeedProgressDialog dialog(count);
    if(dialog.exec())
        return dialog.data;
    else
        return QByteArray();
}


