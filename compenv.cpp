/** @file compenv.cpp
* @brief the computing environment.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about the computing envrionment.
*/

#include "compenv.h"
#include "mainwindow.h"
#include "assert.h"
#include <QComboBox>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QToolTip>
#include <QDir>
#include <QDebug>

void CE::generateBatch(const QString &path,const QModelIndex &index, QWidget *parent)
{
    if(!index.isValid())
        return;

    QDir dir(path);
    assert(dir.exists());


    switch( index.data(CE::PlatformRole).toInt())
    {
    case CE::Posix:
        if( !copyRunshTo(":/file/files/run.sh",dir.absoluteFilePath("run.sh"),index)    )
            QMessageBox::critical(parent,QObject::tr("Error"),QObject::tr("Failed to create the <b>run.sh</b>."),QMessageBox::Ok,QMessageBox::Ok);
        break;
    case CE::PosixPBS:
        if( !copyPBSshTo( ":/file/files/qsub.sh",dir.absoluteFilePath("qsub.sh"),index) ||
            !copyRunshTo(":/file/files/runpbs.sh",dir.absoluteFilePath("run.sh"),index)   )
            QMessageBox::critical(parent,QObject::tr("Error"),QObject::tr("Failed to create the <b>qsub.sh</b> or <b>run.sh</b>."),QMessageBox::Ok,QMessageBox::Ok);
        break;
    case CE::WindowsCmd:
        if( !copyBatTo(":/file/files/run.cmd",dir.absoluteFilePath("run.cmd"),index) )
            QMessageBox::critical(parent,QObject::tr("Error"),QObject::tr("Failed to create the <b>run.cmd</b>."),QMessageBox::Ok,QMessageBox::Ok);
        break;
    }
}

bool CE::copyRunshTo(const QString &source, const QString &path, const QModelIndex &index)
{
    assert(index.isValid());

    QFile file(source);
    if(!file.open(QIODevice::ReadOnly))
        return false;
    QString text = file.readAll();

    text = text.arg(index.data(CE::ThreadNumRole).toString(),compilerCommand("$sds",index));

    QFile out(path);
    if(!out.open(QIODevice::WriteOnly|QIODevice::Truncate) )
        return false;
    return out.write(text.toLocal8Bit()) != -1;
}

bool CE::copyPBSshTo(const QString &source,const QString &path,const QModelIndex &index)
{
    assert(index.isValid());

    QFile file(source);
    if(!file.open(QIODevice::ReadOnly))
        return false;
    QString text = file.readAll();

    text = text.arg(index.data(CE::ThreadNumRole).toString());

    QFile out(path);
    if(! out.open(QIODevice::WriteOnly|QIODevice::Truncate) )
        return false;
    return out.write(text.toLocal8Bit()) != -1;
}

bool CE::copyBatTo(const QString &source, const QString &path, const QModelIndex &index)
{
    assert(index.isValid());

    QFile file(source);
    if(!file.open(QIODevice::ReadOnly))
        return false;
    QString text = file.readAll();

    text.replace("set cores=%cores%","set cores="+index.data(CE::ThreadNumRole).toString().trimmed());
    text.replace("echo %sds%",compilerCommand("%sds%",index));

    QFile out(path);
    if(!out.open(QIODevice::WriteOnly|QIODevice::Truncate) )
        return false;
    return out.write(text.toLocal8Bit()) != -1;
}

std::map<std::string, CE::PlatForm> initializepaltformmap()
{
    std::map<std::string, CE::PlatForm> map;
    map.insert(std::make_pair("windowscmd",CE::WindowsCmd) );
    map.insert(std::make_pair("posix",CE::Posix) );
    map.insert(std::make_pair("posixpbs",CE::PosixPBS) );
    return map;
}

std::map<std::string, CE::PlatForm> &CE::paltformmap()
{
    static std::map<std::string, PlatForm> map = initializepaltformmap();
    return map;
}

QString CE::debugCompilerCommand(const QModelIndex &index)
{
    if(! index.isValid())
        return QString();

    QString cl;
    cl.     append(index.data(CE::CNameRole).toString() ).
            append(' ').append(index.data(CE::COptimizationRole).toString() );

    if( index.data(CE::ThreadNumRole).toInt() != 1)
        QToolTip::showText(QCursor::pos(),"Multithread is banned in the debug mode!");

    cl.     append(' ').append(index.data(CE::CMacroRole).toString() ).append("BD_THREADNUM=1").
            append(' ').append(index.data(CE::CMacroRole).toString() ).append("BD_SEED=").append("static_cast<unsigned>(std::time(0))").
            append(' ').append(index.data(CE::CMacroRole).toString() ).append("EIGEN_DONT_PARALLELIZE");

    cl.     append(' ').append(index.data(CE::CIncludepathRole).toString()).append("../../../source");

    cl.     append(' ').append("main.cpp");

    cl.     append(' ').append(index.data(CE::COutputRole).toString()).append("out.exe");

    return cl;
}

QString CE::compilerCommand(const QString &seeds,const QModelIndex &index)
{
    assert(index.isValid());

    QString cl;
    cl.     append(index.data(CE::CNameRole).toString() ).
            append(' ').append(index.data(CE::COptimizationRole).toString() );

    cl.     append(' ').append(index.data(CE::CMacroRole).toString() ).append("BD_THREADNUM=").append(index.data(CE::ThreadNumRole).toString()).
            append(' ').append(index.data(CE::CMacroRole).toString() ).append("BD_SEED=").append(seeds).
            append(' ').append(index.data(CE::CMacroRole).toString() ).append("EIGEN_DONT_PARALLELIZE");

    cl.     append(' ').append(index.data(CE::CIncludepathRole).toString()).append("../../source");

    cl.     append(' ').append("../main.cpp");

    cl.     append(' ').append(index.data(CE::COutputRole).toString()).append("out.exe");

    return cl;
}

