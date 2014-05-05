/** @file pjanalyser.cpp
* @brief the data-analysing interface of one project.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the data analyser Class, which is
* used for analyse simulation datas and export computing
* tasks.
*/

#include "tabledelegate.h"
#include "pjanalyser.h"
#include "tablefilter.h"
#include "tableview.h"
#include "mdparout.h"
#include "simulationbar.h"
#include "stcode.h"
#include "table.h"
#include "seed.h"
#include "config.h"
#include "outputdialog.h"
#include "item.h"
#include "assert.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QMessageBox>
#include <QSqlQuery>
#include <QAction>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QLabel>
#include <QDir>
#include <QUrl>

PjAnalyser::PjAnalyser(const QDir &root,
                     QWidget *parent)
    : QWidget(parent),
      root(root),
      pm(0)
{
    tableView = new TableView;
    tableView->setItemDelegate(new ResultDelegate(this));   

    //title
    lineEdit = new LineEdit;
    lineEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    label = new QLabel("   Header :");
    label->setObjectName("TitleLabel");

    simExport = new SimulationBar;

    previous = new QToolButton;
    previous->setIcon(QIcon(":/icon/images/previous.png"));
    previous->setToolTip(tr("Previous Page"));
    previous->setText(tr("Previous Page"));
    previous->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    previous->setObjectName("ToolButton");

    refreshAction = new QAction(QIcon(":/icon/images/refresh.png"),tr("&Refresh"),this);
    refreshAction->setEnabled(false);

    //tool bar;
    toolbar = new QToolBar;
    toolbar->setObjectName("TitleBar");
    toolbar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    toolbar->addWidget(previous);
    toolbar->addSeparator();
    toolbar->addActions(tableView->actions());
    toolbar->addAction(refreshAction);
    toolbar->addSeparator();
    toolbar->addWidget(label);
    toolbar->addWidget(lineEdit);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(toolbar);
    layout->addWidget(tableView);
    layout->addWidget(simExport);

    tableFilter = new TableFilter(this);
    tableFilter-> QSortFilterProxyModel::sourceModel();
    tableView->setModel(tableFilter);

    connect(lineEdit, SIGNAL(editingFinished(QString)),this,SLOT(setCurrentHorizontalHeader(QString)) );
    connect(previous,SIGNAL(clicked()),this,SIGNAL(switch2Model()) );
    connect(simExport,SIGNAL(exportTriggered()),this,SLOT(exportSimulationTask()) );
    connect(simExport,SIGNAL(simulationTriggered(bool)),tableView,SLOT(showTimes(bool)) );
    connect(tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(setLineEdit(QModelIndex)) );

    this->setDisabled(true);
}

void PjAnalyser::setCurrentHorizontalHeader(const QString &header)
{
    QModelIndex index = tableView->currentIndex();
    if(! index.isValid())
        return;
    
    assert(tableFilter->sourceModel());
    tableFilter->sourceModel()->setHeaderText(index.column(),header);
}

void PjAnalyser::setLineEdit(const QModelIndex &current)
{
    if(! current.isValid())
        lineEdit->setText(QString(),true);

    assert(tableFilter->sourceModel());

    bool readonly = tableFilter->sourceModel()->headerData(current.column(),Qt::Horizontal,Item::HeaderFlagRole).toInt() &
                    ( Item::Parafiter | Item::Parameter | Item::Times | Item::Id);
    QString text = tableFilter->sourceModel()->headerText(current.column());
    lineEdit->setText(text,readonly);
}

void PjAnalyser::setSourceModel(ParoutModel *sourceModel)
{
    assert(! pm);
    pm = sourceModel;
    tableFilter->setSourceModel(sourceModel->resultModel());
    tableView->setSortingEnabled(true);
    tableView->initializeApperance();
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->setEnabled(true);

    assert(sourceModel->resultModel());
    connect(refreshAction,SIGNAL(triggered()), sourceModel->resultModel(),SLOT(calculateAll()) );
    connect(refreshAction,SIGNAL(triggered()), sourceModel->resultModel(),SLOT(refreshAll()) );
    refreshAction->setEnabled(true);
}

void PjAnalyser::setReadOnly(bool ro)
{
    lineEdit->setDisabled(ro);
    tableView->setEditTriggers(ro ? QAbstractItemView::NoEditTriggers : QAbstractItemView::DoubleClicked|QAbstractItemView::AnyKeyPressed);
    simExport->setDisabled(ro);
}

bool PjAnalyser::lockParameter()
{
    for (int row=0; row< tableFilter->rowCount()-1; ++row)
    {
        assert(tableFilter->sourceModel());
        int times = tableFilter->index(row,tableFilter->sourceModel()->parameterColumnCount()).data().toInt();

        if(times < 1 )
            continue;

        QString key = tableFilter->headerData(row,Qt::Vertical).toString();
        assert(!key.isEmpty());
        if(! root.exists(key) && !root.mkdir(key))
            return false;
        if(! tableFilter->setHeaderData(row,Qt::Vertical,Item::SimData,Item::DataRole) )
            return false;

        QFont font = tableFilter->headerData(row,Qt::Vertical,Qt::FontRole).value<QFont>();
        font.setItalic(true);
        if(! tableFilter->setHeaderData(row,Qt::Vertical,font,Qt::FontRole) )
            return false;
    }
    return true;
}

void PjAnalyser::exportSimulationTask()
{   
    assert(pm);
    OutputDialog dialog(this);
    if(! dialog.exec())
        return;

    QString path = dialog.getPath();
    assert(! path.isEmpty()) ;

    if(! Codist::copyto(path) )
    {
        QMessageBox::critical(this,tr("Error"), tr("Failed to export the <b>source</b> folder."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if(! lockParameter())
        return;

    QStringList idList;
    int seednum = tableFilter->generateMain(root.absoluteFilePath("main.cpp"),path, idList,this);
    if(seednum == 0 || idList.isEmpty()) return;

    QString name = QFileInfo(root.absolutePath()).fileName();
    if(name.isEmpty())
        return;

    //open time.stamp.
    QFile fv(root.absoluteFilePath("time.stamp"));
    if(! fv.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,tr("Error"), "Failed to read the <b>time.stamp</b> file.", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    const QString version = fv.readAll();

    //open project identification.
    QFile fpi(QDir(path).absoluteFilePath(name+".pi"));
    if( !fpi.open(QIODevice::WriteOnly) )
    {
        QMessageBox::critical(this,tr("Error"), "Failed to generate the <b>" + name + ".pi</b> file.", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }   
    const QString outstr = identification(name,pm->outputs(),idList,version);
    fpi.write(outstr.toLatin1());
    fpi.close();

    generateBatch(path, simExport->currentEnvInfo());

    if(! seedsource(path) (seednum*10) )
    {
        QMessageBox::critical(this,tr("Error"), tr("Failed to create the <b>seeds</b> and <b>mark</b> files."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void PjAnalyser::generateBatch(const QString &path, const QFileInfo &envInfo)
{
    if(!envInfo.isFile())
        return;

    QDir dir(path);
    assert(dir.exists());

   QFileInfoList _names = envInfo.absoluteDir().entryInfoList(QStringList(envInfo.baseName().append(".*")),QDir::Files);
   foreach (const QFileInfo &it, _names) {
       if(! QFile::copy(it.absoluteFilePath(),dir.absoluteFilePath(it.fileName())) )
       {
           QMessageBox::critical(this,QObject::tr("Error"),"Failed to create "+it.fileName(),QMessageBox::Ok,QMessageBox::Ok);
           break;
       }
   }
}

