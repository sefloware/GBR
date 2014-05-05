/** @file project.cpp
* @brief the project widget.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the project widget composed of
* modeler and analyser widget.
*/

#include "project.h"
#include "pjanalyser.h"
#include "pjmodeler.h"
#include "mdeditor.h"
#include "mdparout.h"
#include "mainwindow.h"
#include "table.h"
#include "tableview.h"
#include "assert.h"
#include <QToolButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QCloseEvent>

Project::Project(const QString &path, QTabWidget *parent) :
    QStackedWidget(parent),
    dir(path),
    parentTab(parent)
{   
    this->setWindowFilePath( path);

    //connect to the project database.
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",dir.absolutePath());
    db.setDatabaseName(dir.absoluteFilePath("data.db") );
    if (!db.open())
    {
        QMessageBox::critical(this, tr("Error"), "Failed to establish a database connection to the project database:\n"+ dir.absoluteFilePath("data.db"), QMessageBox::Ok, QMessageBox::Ok);
        setDisabled(true);
        return;
    }

    pjmd = new PjModeler(dir,0,db);
    pjal = new PjAnalyser(dir);
    pjal->setSourceModel(pjmd->paroutModel);

    addWidget(pjmd);
    addWidget(pjal);

    connect(pjmd->paroutModel,SIGNAL(addFilterAndHideColumn(int,QVariant)),pjal->tableFilter,SLOT(addFilter(int,QVariant)) );
    connect(pjmd->paroutModel,SIGNAL(addFilterAndHideColumn(int,QVariant)),pjal->tableView,SLOT(initializeApperance()) );

    connect(pjmd->paroutModel,SIGNAL(deleteFilterAndShowColumn(int)),pjal->tableFilter,SLOT(deleteFilter(int)) );
    connect(pjmd->paroutModel,SIGNAL(deleteFilterAndShowColumn(int)),pjal->tableView,SLOT(initializeApperance()) );

    connect(pjmd->paroutModel,SIGNAL(initializeResultViewApperance()),pjal->tableView,SLOT(initializeApperance()) );
    connect(pjmd,SIGNAL(modificationChanged(bool)),this,SLOT(slotModificationChanged(bool)) );

    connect(pjmd,SIGNAL(switch2Analyse()),this,SLOT(switch2Analyse()) );
    connect(pjal,SIGNAL(switch2Model()),this,SLOT(switch2Model()) );
}

void Project::refresh()
{
    pjmd->paroutModel->resultModel()->calculateAll();
}

void Project::updateEditorEnvironment()
{ pjmd->editor->updateEnvModel(); }

Project::~Project()
{
    delete pjmd;
    delete pjal;
    parentTab->removeTab( parentTab->indexOf(this));
    QSqlDatabase::removeDatabase(dir.absolutePath());
}

void Project::switch2Model()
{ setCurrentWidget(pjmd); }
void Project::switch2Analyse()
{ setCurrentWidget(pjal); }

void Project::slotModificationChanged(bool modified)
{
    if( isWindowModified() == modified )
        return;

    QStackedWidget::setWindowModified( modified);
    int index = parentTab->indexOf(this);
    QString title = windowTitle();
    title.replace("[*]",modified ? "*" : "");
    parentTab-> setTabText(index, title);

    pjal->setReadOnly(modified);
}
