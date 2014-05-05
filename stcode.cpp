/** @file stcode.cpp
* @brief the source code area  in the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the source code area which
* is composed of the title, the bar, the data model
* for the souce code resource and the view.
*/

#include "stcode.h"
#include "inputdialog.h"
#include "cppreader.h"
#include "cppobjlist.h"
#include "config.h"
#include "editor1.h"
#include "assert.h"
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QToolBar>
#include <QTreeView>
#include <QLineEdit>
#include <QAction>
#include <QLabel>

CodeModel::CodeModel(const QSet<QString> &modifiedSourceSet, QObject *parent) :
    QFileSystemModel(parent),
    modifiedSourceSet(modifiedSourceSet),
    icon(":/icon/images/hfile.png")
{ this->setReadOnly(false); }

QVariant CodeModel::data(const QModelIndex &index, int role) const
{
    if(index.column())
        return QFileSystemModel::data(index,role);

    switch (role) {
    case Qt::DisplayRole:
    {
        const QString path = QFileSystemModel::data(index,QFileSystemModel::FilePathRole).toString();
        QString name = QFileSystemModel::data(index, Qt::EditRole).toString();
        if(modifiedSourceSet.contains(path))
            return name.append('*');
        else
            return name;
    }
    case Qt::DecorationRole:
        return icon;
    default:
        return QFileSystemModel::data(index,role);
    }
}
Qt::ItemFlags CodeModel::flags(const QModelIndex & index) const
{
    Qt::ItemFlags fg = QFileSystemModel::flags(index);
    const QString path = index.data(QFileSystemModel::FilePathRole).toString();
    if( modifiedSourceSet.contains(path) )
            fg ^= Qt::ItemIsEditable;
    return fg;
}

Codist::Codist(QWidget *parent) :
    QWidget(parent)
{
    QString path = absolutePath(SourceFolder);
    if(path.isEmpty())
    {
        setDisabled(true);
        return;
    }

    QLabel *title = new QLabel("Source");
    title->setObjectName("TitleLabel");
    title->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    bar = new QToolBar;
    bar->setObjectName("TitleBar");
    bar->addWidget(title);
    newAction =  bar->addAction(QIcon(":/icon/images/add.png"),"New",this,SLOT(newSource()) );
    actionDelete = bar->addAction(QIcon(":/icon/images/delete.png"),"Delete",this,SLOT(removeSource()) );
    explorAction = new QAction(QIcon(":/icon/images/explore.png"),tr("Open Folder"),this);
    bar->addSeparator();
    bar->addAction(explorAction);
    connect(explorAction,SIGNAL(triggered()), this,SLOT(explorCurrent()));

    renameAction = new QAction(QIcon(":/icon/images/rename.png"),tr("&Rename"),this);
    renameAction->setShortcut(QKeySequence("F2"));
    connect(renameAction,SIGNAL(triggered()),this,SLOT(rename()) );
    renameAction->setEnabled(false);

    model = new CodeModel(modifiedSourceSet,this);
    model->setFilter(QDir::Files);
    model->setNameFilters(QStringList("*.h"));
    model->setNameFilterDisables(false);

    view = new QTreeView;
    view->setModel(model);
    view->setRootIndex(model->setRootPath(path));
    for (int i=1; i<model->columnCount(); ++i)
        view->hideColumn(i);
    view->setItemsExpandable(false);
    view->setIndentation(0);
    view->setHeaderHidden(true);
    view->addAction(newAction);
    view->addAction(renameAction);
    view->addAction(actionDelete);
    QAction *seperator = new QAction(this);
    seperator->setSeparator(true);
    view->addAction(seperator);
    view->addAction(explorAction);
    view->setContextMenuPolicy(Qt::ActionsContextMenu);
    registerAllToSource();
    connect(model,SIGNAL(directoryLoaded(QString)),this,SLOT(resizeViewWidth()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(bar);
    layout->addWidget(view);
    this->setLayout(layout);

    connect(view->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(updateAction(QModelIndex)) );
    connect(view->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(emitCodePath(QModelIndex)) );
    connect(model,SIGNAL(fileRenamed(QString,QString,QString)),this,SLOT(slotFileRenamed(QString,QString,QString)) );
}

void Codist::slotFileRenamed(const QString &path, const QString &oldName, const QString &newName)
{
    const QString oldPath = path + "/" + oldName;
    const QString newPath = path + "/" + newName;

    if( modifiedSourceSet.contains(oldPath) )
    {
        modifiedSourceSet.remove(oldPath);
        modifiedSourceSet.insert(newPath);
    }

    if(Editor1::source().contains(oldName))
    {
        QStandardItemModel *model = Editor1::source().value(oldName);
        Editor1::source().remove(oldName);
        Editor1::source().insert(newName,model);
    }

    emit updateEditorEnvironment();
}

void Codist::updateAction(const QModelIndex &current)
{
    explorAction->setEnabled(current.isValid());

    QString path = current.data(QFileSystemModel::FilePathRole).toString();

    bool enable = current.isValid();

    actionDelete->setEnabled(enable);

    if( modifiedSourceSet.contains(path))
        enable = false;
    renameAction->setEnabled(enable);
}

void Codist::rename()
{  view->edit(view->currentIndex()); }

void Codist::explorCurrent()
{ QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath(SourceFolder)));  }

void Codist::clearSelection()
{
    view->selectionModel()->clearSelection();
    view->selectionModel()->clearCurrentIndex();
}

bool Codist::isModified() const
{
    if(modifiedSourceSet.empty() )
        return false;

    const QFileInfoList list = model->rootDirectory().entryInfoList(model->nameFilters(),model->filter());
    foreach (const QFileInfo &it, list)
        if( modifiedSourceSet.contains(it.absoluteFilePath()) )
            return true;

    return false;
}

void Codist::setModification(const QString &file, bool modified)
{
    assert(! file.isEmpty());
    if(modified)    modifiedSourceSet.insert(file);
    else
    {
        modifiedSourceSet.remove(file);
        emit updateEditorEnvironment();
    }
    view->viewport()->update() ;
}

void Codist::emitCodePath(const QModelIndex &current)
{ emit currentChanged( model->filePath(current) ); }

void Codist::registerAllToSource()
{
    QFileInfoList infos = model->rootDirectory().entryInfoList(model->nameFilters(),model->filter());
    foreach (const QFileInfo &it, infos) {
        registerToSource(it);
    }
}

void Codist::resizeViewWidth()
{ view->resizeColumnToContents(0);     }

void Codist::registerToSource(const QFileInfo &fileInfo)
{
    QFile file(fileInfo.absoluteFilePath().append('c'));
    if( !file.exists())
        file.setFileName(fileInfo.absoluteFilePath());
    if( ! file.open(QIODevice::ReadOnly))
        return;

    const QString text = file.readAll();
    CppReader reader(text);
    CppObjList items;
    items << reader;
    if(items.isEmpty())
        return;

    items.readDescription(text);

    QStandardItemModel *model = new QStandardItemModel;
    foreach (QStandardItem *it, items) {
        model->appendRow(it);
    }
    model->setHorizontalHeaderLabels(QStringList(fileInfo.fileName()));
    Editor1::source().insert(fileInfo.fileName(),model);
}

void Codist::newSource()
{
    bool ok;
    QRegExp regExp("^[a-zA-Z_]\\w*\\.h$");
    QRegExpValidator v(regExp, 0);
    QString fileName = InputDialog::getText(this,tr("New Source"),tr("File Name(*.h):"),QString(),&v,QString(),&ok);
    if(!ok) return;

   if(model->rootDirectory().exists(fileName))
   {
       QMessageBox::warning(this,tr("Warning"),"<b>" + fileName + "</b> has been used.",QMessageBox::Ok,QMessageBox::Ok);
       return;
   }

   const QString filePath = model->rootDirectory().absoluteFilePath(fileName);
   QFile file(":/file/files/fun0.cpp");
   ok = file.open(QIODevice::ReadOnly);
   assert(ok);

   QString text = file.readAll();
   QString defname = fileName.replace('.','_');

   QFile newfile(filePath);

   if(! newfile.open(QIODevice::WriteOnly))
   {
       QMessageBox::critical(this,tr("Error"),"Failed to create the <b>"+fileName+"</b>.",QMessageBox::Ok,QMessageBox::Ok);
       return;
   }

   newfile.write(text.arg(defname).toLatin1());
}

bool Codist::removeSource()
{
    QModelIndex index = view->currentIndex();
    if(!index.isValid()) return false;

    QFile file( model->filePath(index).append('c'));
    if(file.exists() && !file.remove() )
        return false;
    Editor1::source().remove(index.data(QFileSystemModel::FileNameRole).toString());
    bool ok = model->remove(index);
    emit updateEditorEnvironment();
    return ok;
}

bool Codist::recurseCopyDir(QDir &s,QDir &d)
{
    QStringList sl = s.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    if(sl.count() == 0) return true;
    foreach (const QString &it, sl) {
        QFileInfo info(s.absoluteFilePath(it));
        if(info.isSymLink()) continue;

        if(info.isFile())
        {
            assert(!d.exists(it));
            if(!QFile::copy(s.absoluteFilePath(it),d.absoluteFilePath(it)))
                return false;
        }
        else
        {
            if( (!(d.mkdir(it)) || !d.cd(it)) || !s.cd(it))
                return false;
            if(!recurseCopyDir(s,d)) return false;
            if(!s.cdUp() || !d.cdUp()) return false;
        }
    }
    return true;
}

bool Codist::copyto(const QString &path)
{
    if(path.isEmpty())
        return false;

    QString sourcePath = absolutePath(SourceFolder);
    if(sourcePath.isEmpty()) return false;

    QDir s(sourcePath);
    QDir d(path);
    if(d.exists("source") && (! d.cd("source") || ! d.removeRecursively() ))
        return false;
    if(! d.mkdir("source") || !d.cd("source"))
        return false;
    return recurseCopyDir(s,d);
}
