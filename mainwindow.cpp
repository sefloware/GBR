/** @file mainwindow.cpp
* @brief the main window
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the main window Class.
*/

#include "mainwindow.h"
#include "tableview.h"
#include "project.h"
#include "stproject.h"
#include "stcode.h"
#include "startpage.h"
#include "pjmodeler.h"
#include "pjanalyser.h"
#include "compenv.h"
#include "cppkeywords.h"
#include "config.h"
#include "assert.h"
#include <QFileDialog>
#include <QFileSystemModel>
#include <QDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    this->setWindowOpacity(0);
    this->setWindowIcon(QIcon(":/icon/images/GBR.png"));

    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);

    startPage = new StartPage;
    tabWidget-> QTabWidget::addTab (startPage, "Start");
    tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->hide();

    connect(startPage->projist,SIGNAL(aboutToOpenPath(QString)), this,SLOT(openProject(QString)) );
    connect(tabWidget,SIGNAL(tabCloseRequested(int)), this,SLOT(closeProject(int)));
    connect(startPage->projist,SIGNAL(updateResultTable(QString)), this,SLOT(updateResultTable(QString)) );
    connect(startPage->codist,SIGNAL(updateEditorEnvironment()), this, SLOT(updateEditorEnvironment()) );

    setCentralWidget(tabWidget);
    readRunEnv();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    for(int i = tabWidget->count()-1; i > 0; --i)
        if(! closeProject(i))
        {
            event->ignore();
            return;
        }

    if (startPage->isWindowModified())
    {
        if(tabWidget->currentWidget() != startPage)
            tabWidget->setCurrentWidget(startPage);
        int answer = QMessageBox::warning(this,tr("Unsaved Changes Warning"), tr("The current page has been changed!"), QMessageBox::Ignore | QMessageBox::Cancel, QMessageBox ::Ignore);
        if (answer == QMessageBox::Cancel)
        {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::openProject(const QString &path)
{
    assert(! path.isEmpty());
    Project *project = new Project(path,tabWidget);
    QString title = project->windowTitle();
    title.remove("[*]");
    int index = tabWidget-> addTab( project, title);
    tabWidget-> setCurrentIndex( index);
}

void MainWindow::updateEditorEnvironment()
{
    for(int i=1; i<tabWidget->count(); ++i)
    {
        Project *project = static_cast<Project *>(tabWidget->widget(i) );
        if(project)
            project->updateEditorEnvironment();
    }
}

void MainWindow::updateResultTable(const QString &path)
{
    assert(! path.isEmpty());
    for(int i=1; i< tabWidget->count(); ++i)
    {
        QWidget *widget = tabWidget->widget(i);
        assert(widget);
        Project *project = static_cast<Project *> (widget);
        if(project->windowFilePath() == path)
        {
            project->refresh();
            return;
        }
    }
}

bool MainWindow::closeProject(int index)
{
    QWidget *currentWidget = tabWidget->widget(index);
    assert(currentWidget && currentWidget != startPage);

    if(currentWidget != tabWidget->currentWidget())
        tabWidget->setCurrentWidget(currentWidget);

    Project *project = static_cast<Project *>(currentWidget );
    if( project->isWindowModified() )
    {
        int answer = QMessageBox::warning(this,tr("Unsaved Changes Warning"), tr("The current page has been changed!"), QMessageBox::Ignore | QMessageBox::Cancel, QMessageBox ::Cancel);
        if (answer == QMessageBox::Cancel)
            return false;
    }

    project->pjmd->saveTemporary();

    const QString path = project->windowFilePath();
    startPage->projist->removeMark(path);

    delete project;

    return true;
}
