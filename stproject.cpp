/** @file stproject.cpp
* @brief the project area  in the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the project area which is
* composed of the title, the bar, the data model
* for the project sets and the view.
*/

#include "stproject.h"
#include "config.h"
#include "outputdialog.h"
#include "inputdialog.h"
#include <QToolBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QAction>
#include <QMouseEvent>
#include <fstream>
#include <QDesktopServices>
#include <QDebug>

ProjectModel::ProjectModel(const QSet<QString> &openedProjectSet,const QSet<QString> &modifiedProjectSet,QObject *parent) :
    QFileSystemModel(parent),
    modifiedProjectSet(modifiedProjectSet),
    openedProjectSet(openedProjectSet),
    openedIcon(":/icon/images/opendproject.png"),
    closedIcon(":/icon/images/closedproject.png")
{ this->setReadOnly(false);}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
    if(index.column() )
        return QFileSystemModel::data(index,role);

    switch (role)
    {
    case Qt::DisplayRole:
    {
        const QString path = QFileSystemModel::data(index,QFileSystemModel::FilePathRole).toString();
        QString name = QFileSystemModel::data(index, Qt::DisplayRole).toString();
        if(modifiedProjectSet.contains(path))
            return name.append('*');
        else
            return name;
    }
    case Qt::DecorationRole:
    {
        const QString path = QFileSystemModel::data(index,QFileSystemModel::FilePathRole).toString();
        if(openedProjectSet.contains(path))
            return openedIcon;
        else
            return closedIcon;
    }
    default:
        return QFileSystemModel::data(index,role);
    }
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags fg = QFileSystemModel::flags(index);
    const QString path = index.data(QFileSystemModel::FilePathRole).toString();
    if( openedProjectSet.contains(path) ||
            modifiedProjectSet.contains(path) )
            fg ^= Qt::ItemIsEditable;
    return fg;
}


Projist::Projist(QWidget *parent) :
    QWidget(parent)
{
    QString path = absolutePath(ProjectFolder);
    if(path.isEmpty())
    {
        setDisabled(true);
        return;
    }

    QLabel *title = new QLabel("Project");
    title->setObjectName("TitleLabel");
    title->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    model = new ProjectModel(openedProjectSet,modifiedProjectSet,this);
    model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);     

    view = new QTreeView;
    view->setModel(model);
    view->setRootIndex(model->setRootPath(path));
    for (int i=1; i<model->columnCount(); ++i)
        view->hideColumn(i);
    view->setItemsExpandable(false);
    view->setIndentation(0);
    view->setHeaderHidden(true);
    view->setEditTriggers(QAbstractItemView::EditKeyPressed);

    connect(model,SIGNAL(directoryLoaded(QString)),this,SLOT(resizeViewWidth()) );
    connect(view->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(updateAction(QModelIndex)) );

    bar = new QToolBar;
    bar->addWidget(title);
    bar->setObjectName("TitleBar");
    openAction = bar->addAction(QIcon(":/icon/images/open.png"),tr("&Open"),this,SLOT(openProject()));
    openAction->setEnabled(false);
    bar->addSeparator();
    newAction =  bar->addAction(QIcon(":/icon/images/add.png"),tr("&New"),this,SLOT(newProject()));
    removeAction =  bar->addAction(QIcon(":/icon/images/delete.png"),tr("&Delete"),this,SLOT(removeProject()) );
    removeAction->setEnabled(false);
    bar->addSeparator();
    importAction = bar->addAction(QIcon(":/icon/images/importsimulation.png"),tr("Import Simulations"),this,SLOT(importSimulations()) );

    deriveAction = new QAction(QIcon(":/icon/images/derive.png"),tr("&Derive"),this);
    renameAction = new QAction(QIcon(":/icon/images/rename.png"),tr("&Rename"),this);
    connect(deriveAction,SIGNAL(triggered()),this,SLOT(derive()) );
    connect(renameAction,SIGNAL(triggered()),this,SLOT(rename()) );
    renameAction->setEnabled(false);
    deriveAction->setEnabled(false);

    explorAction = new QAction(QIcon(":/icon/images/explore.png"),tr("Open Folder"),this);
    connect(explorAction,SIGNAL(triggered()), this,SLOT(explorCurrent()));

    view->addAction(openAction);
    view->addAction(newAction);
    view->addAction(removeAction);
    view->addAction(renameAction);
    view->addAction(deriveAction);
    QAction *seperator = new QAction(this);
    seperator->setSeparator(true);
    view->addAction(seperator);
    view->addAction(explorAction);
    view->setContextMenuPolicy(Qt::ActionsContextMenu);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(bar);
    layout->addWidget(view);

    connect(view->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(emitProjectPath(QModelIndex)));
    connect(model,SIGNAL(fileRenamed(QString,QString,QString)),this,SLOT(slotFileRenamed(QString,QString,QString)) );
}

void Projist::slotFileRenamed(const QString &path, const QString &oldName, const QString &newName)
{
    const QString oldPath = path + "/" + oldName;
    const QString newPath = path + "/" + newName;

    if( openedProjectSet.contains(oldPath) )
    {
        openedProjectSet.remove(oldPath);
        openedProjectSet.insert(newPath);
    }

    if( modifiedProjectSet.contains(oldPath) )
    {
        modifiedProjectSet.remove(oldPath);
        modifiedProjectSet.insert(newPath);
    }
}

void Projist::emitProjectPath(const QModelIndex &index)
{ emit currentChanged(model->filePath(index)); }

void Projist::updateAction(const QModelIndex &current)
{
    explorAction->setEnabled(current.isValid());

    QString path = current.data(QFileSystemModel::FilePathRole).toString();

    bool enable = current.isValid() &&
            ! openedProjectSet.contains(path);

    removeAction->setEnabled(enable);
    openAction->setEnabled(enable);
    deriveAction->setEnabled(enable);

    if( modifiedProjectSet.contains(path))
        enable = false;
    renameAction->setEnabled(enable);
}

void Projist::setModification(const QString &project, bool modified)
{
    assert(! project.isEmpty());
    if(modified)    modifiedProjectSet.insert(project);
    else            modifiedProjectSet.remove(project);
    view->viewport()->update() ;
}

void Projist::newProject()
{
    QRegExp rx("^[a-zA-Z_]\\w*$");
    QValidator *validator = new QRegExpValidator(rx, this);
    QString _name = InputDialog::getText(this,"New Project","Name: ",QString(),validator);
    if(_name.isEmpty())
        return;
    if(model->rootDirectory().exists(_name) )
    {
        const QString message = "<i>%1</i> has been used.";
        QMessageBox::critical(this,tr("Error"),message.arg(_name), QMessageBox::Ok);
        return;
    }
    model->rootDirectory().mkdir(_name);
}

void Projist::removeProject()
{
    QModelIndex index = view->currentIndex();
    if(! index.isValid())
        return;
    model->remove(index);
}

void Projist::removeMark(const QString &path)
{
    openedProjectSet.remove(path);
    updateAction(view->currentIndex());
    view->viewport()->update() ;
}

void Projist::openProject()
{
    const QModelIndex index = view->currentIndex();
    if (! index.isValid())
        return;

    QString filepath = model->filePath(index);
    if( filepath.isEmpty() || openedProjectSet.contains(filepath))
        return;
    openedProjectSet.insert(filepath);
    updateAction(view->currentIndex());
    view->viewport()->update() ;

    emit aboutToOpenPath(filepath);
}

void Projist::explorCurrent()
{ QDesktopServices::openUrl(QUrl::fromLocalFile(view->currentIndex().data(QFileSystemModel::FilePathRole).toString() ) );  }


void Projist::rename()
{
    view->edit(view->currentIndex());
}

void Projist::derive()
{
    QRegExp rx("^[a-zA-Z_]\\w*$");
    QValidator *validator = new QRegExpValidator(rx, this);
    QString _name = InputDialog::getText(this,"New Project","Name: ",QString(),validator);
    if(_name.isEmpty())
        return;
    if(model->rootDirectory().exists(_name) )
    {
        const QString message = "<i>%1</i> has been used.";
        QMessageBox::critical(this,tr("Error"),message.arg(_name), QMessageBox::Ok);
        return;
    }

    model->rootDirectory().mkdir(_name);
    const QString path = model->rootDirectory().absoluteFilePath(_name);
    const QString source = view->currentIndex().data(QFileSystemModel::FilePathRole).toString();
    QFile::copy(source+"/main.cpp",path+"/main.cpp");
}

void Projist::resizeViewWidth()
{
    view->resizeColumnToContents(0);
}

void Projist::clearSelection()
{
    view->selectionModel()->clearSelection();
    view->selectionModel()->clearCurrentIndex();
}

bool Projist::isModified() const
{
    if( modifiedProjectSet.empty())
        return false;


    const QFileInfoList list = model->rootDirectory().entryInfoList(model->nameFilters(),model->filter());
    foreach (const QFileInfo &it, list)
        if( modifiedProjectSet.contains(it.absoluteFilePath()) )
            return true;

    return false;
}

bool Projist::importStfile(const std::ifstream &source, std::ofstream &destination)
{
    if(!source || !destination)
        return false;

    destination << source.rdbuf();
    return !destination.fail();
}

bool Projist::importFile(const QString &source, const QDir &destination)
{
    QFileInfo info(source);

    if( !info.exists())
        return true;

    QString sn = info.fileName();
    if(sn.contains('*') || ! sn.contains('.'))
        return false;

    if(sn.endsWith(".st"))
    {
        if(! destination.exists(sn))
            return QFile::copy(source,destination.absoluteFilePath(sn));

        QByteArray tmp1 = info.absoluteFilePath().toLocal8Bit();
        std::ifstream fsour(tmp1.data());
        QByteArray tmp2 = destination.absoluteFilePath(sn).toLocal8Bit();
        std::ofstream fdest(tmp2.data(),std::ios::app);
        return importStfile(fsour,fdest);
    }
    else
    {
        int pos = sn.lastIndexOf('.');
        sn.insert(pos,"_*");
        QStringList list = destination.entryList(QStringList(sn),QDir::Files,QDir::Name);
        int count = 0;
        if(! list.isEmpty())
        {
            bool ok;
            count = list.last().mid(pos+1,list.last().lastIndexOf('.')-(pos+1) ).toInt(&ok);
            if(! ok) return false;
            ++count;
        }
        sn.replace('*',QString::number(count));
        return QFile::copy(source,destination.absoluteFilePath(sn)) ;
    }
}

void Projist::importSimulations()
{
    //get path
    const QString pipath = QFileDialog::getOpenFileName(this,tr("Import Simulations Dialog"), OutputDialog::defaultPath() ,tr("Project Identification (*.pi)" ) );
    if(pipath.isEmpty())
        return;
    //open info;
    QFile fpi(pipath);
    if(! fpi.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,tr("Error"),"Failed to open the <b>" + pipath + "</b>", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    QString text = fpi.readAll();
    fpi.close();

    QMap<QString,QString> map = parseIdentification(text);
    if(map.isEmpty())
    {
        QMessageBox::critical(this,tr("Error"),"This <b>" + QFileInfo(pipath).fileName() + "</b> is invalid whose content is:<br>" + text, QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    //open time.stamp.
    QDir destDir( absolutePath(ProjectFolder) );

    if(! destDir.cd(map.value("Project")) )
    {
        QMessageBox::critical(this,tr("Error"),"Failed to access to the project <b>" + map.value("Project") + "</b>.", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QFile fv(destDir.absoluteFilePath("time.stamp"));
    if(! fv.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,tr("Error"), "Failed to open the file :\n" + destDir.absoluteFilePath("time.stamp"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    const QString version = fv.readAll();
    fv.close();

    if(map.value("Time_stamp") != version)
    {
        int answer = QMessageBox::warning(this,tr("Warning"),"The time stamp of the" + QFileInfo(pipath).fileName() + "is inconsistent with that of the project <b>" + map.value("Time_stamp") + "</b>.<br><br><b>" + QFileInfo(pipath).fileName() + "</b>\'s content is:<br>" + text, QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel);
       if( answer == QMessageBox::Cancel)
           return;
    }
    else
    {
        int answer = QMessageBox::information(this,tr("Information"),"The <b>" + QFileInfo(pipath).fileName() + "</b>\'s content is:<br>" + text, QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok);
        if( answer == QMessageBox::Cancel)
            return;
    }

    QDir sourDir(QFileInfo(pipath).absolutePath());

    QStringList miss;
    const QStringList idlist = map.value("ID").split(',',QString::SkipEmptyParts);
    foreach (const QString &it, idlist)
    {
        if( ! sourDir.cd(it)  )
        {
            miss << "The group of parameters of the Id <b>" + it + "</b> has not been simulated yet!";
            continue;
        }

        if(! destDir.cd(it) )
        {
            miss << "failed to import the <b>" + sourDir.absolutePath() + "</b>!";
            sourDir.cdUp();
            continue;
        }

        const QStringList sourceList = sourDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);

        foreach (const QString &subit, sourceList)
        {
            if(! subit.contains(QRegExp("^\\d+$")) )
                continue;

            if(! sourDir.cd(subit) )
            {
                QMessageBox::critical(this,tr("Error"), "Failed to access to the directory:\n" + sourDir.absoluteFilePath(subit), QMessageBox::Ok,QMessageBox::Ok);
                return;
            }

            foreach (const QString &outit,  map.value("Output").split(',',QString::SkipEmptyParts))
            {
                if(! importFile(sourDir.absoluteFilePath(outit),destDir ) )
                    miss << "failed to import the <b>" + sourDir.absoluteFilePath(outit) + "</b>!";
            }
            sourDir.cdUp();
        }

        sourDir.cdUp();
        destDir.cdUp();
    }

    QString message("<b>Importation completes</b><br><br>");
    if(! miss.isEmpty())
        message.append("the missed items:<br>").append(miss.join("<br>")).append("<br><br>");

    message.append("remove the folder: ").append( sourDir.path()).append( "?");

    int answer = QMessageBox::information(this,"information",message,QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes );
    if( answer == QMessageBox::Yes && sourDir.exists())
        sourDir.removeRecursively();

    if(openedProjectSet.contains(destDir.absolutePath()))
        emit updateResultTable(destDir.absolutePath());
}
