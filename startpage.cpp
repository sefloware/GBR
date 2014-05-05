/** @file startpage.cpp
* @brief the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the start page (the opening page of the soft).
*/

#include "stprojecteditor.h"
#include "richtextwidget.h"
#include "startpage.h"
#include "stcodeeditor.h"
#include "stproject.h"
#include "stcode.h"
#include "assert.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QMouseEvent>
#include <QTreeView>
#include <QLabel>

StartPage::StartPage(QWidget *parent) :
    QSplitter(Qt::Horizontal,parent)
{
    projist = new Projist;
    codist = new Codist;

    QSplitter *vs = new QSplitter(Qt::Vertical);
    vs->addWidget(projist);
    vs->addWidget(codist);

    welcomeWidget = new RichTextWidget;
    const QString welcome = ":/file/files/welcome.html";
    welcomeWidget->fileOpen(welcome);
    welcomeWidget->setWindowFilePath(welcome);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(welcomeWidget);

    this->addWidget(vs);
    this->addWidget(stackedWidget);

    projist->view->viewport()->installEventFilter(this);
    codist->view->viewport()->installEventFilter(this);

    connect(projist->model,SIGNAL(fileRenamed(QString,QString,QString)),this,SLOT(renameProject(QString,QString,QString)) );
    connect(codist->model,SIGNAL(fileRenamed(QString,QString,QString)),this,SLOT(renameCode(QString,QString,QString)) );

    connect(projist,SIGNAL(currentChanged(QString)),this,SLOT(displayProject(QString)) );
    connect(codist,SIGNAL(currentChanged(QString)),this,SLOT(displayCode(QString)) );

    connect(codist,SIGNAL(updateEditorEnvironment()),this,SLOT(updateEditorEnvironment()) );
}

bool StartPage::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::MouseButtonPress)
        return QWidget::eventFilter(obj, event);

    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

    if( obj == projist->view->viewport())
    {
        codist->clearSelection();

        QModelIndex index = projist->view->indexAt(mouseEvent->pos());
        if(! index.isValid() )
        {
            projist->clearSelection();
            stackedWidget->setCurrentWidget(welcomeWidget);
        }
    }
    else if( obj == codist->view->viewport())
    {
        projist->clearSelection();

        QModelIndex index = codist->view->indexAt(mouseEvent->pos());
        if(! index.isValid() )
        {
            codist->clearSelection();
            stackedWidget->setCurrentWidget(welcomeWidget);
        }
    }

    return QWidget::eventFilter(obj, event);
}

void StartPage::renameProject(const QString &path,const QString &oldName, const QString &newName)
{
    const QString oldPath = path + "/" + oldName;
    const QString newPath = path + "/" + newName;

    if(! mapproj.contains(oldPath))
        return;

    ProjectEditor *projectWidget = static_cast<ProjectEditor *>(mapproj.value(oldPath));
    projectWidget->setWindowFilePath(newPath);

    mapproj.remove(oldPath);
    mapproj.insert(newPath,projectWidget);
}

void StartPage::updateEditorEnvironment()
{
    foreach (CodeEditor *it, mapcode) {
        it->updateEditorEnvironment();
    }
}

void StartPage::renameCode(const QString &path,const QString &oldName, const QString &newName)
{
    const QString oldPath = path + "/" + oldName;
    const QString newPath = path + "/" + newName;

    if(! mapcode.contains(oldPath))
        return;

    CodeEditor *codeWidget = static_cast<CodeEditor *>(mapcode.value(oldPath));
    codeWidget->setWindowFilePath(newPath);

    mapcode.remove(oldPath);
    mapcode.insert(newPath,codeWidget);
}

void StartPage::displayProject(const QString &projectPath)
{
    if( projectPath.isEmpty() )
    {
        QWidget *widget = stackedWidget->currentWidget();
        assert(widget);
        const QString path = widget->windowFilePath();
        if(! mapproj.contains(path)) return;

        stackedWidget->setCurrentWidget(welcomeWidget);
        return;
    }

    if(mapproj.contains(projectPath) )
    {
        stackedWidget->setCurrentWidget(mapproj.value(projectPath));
        return;
    }

    ProjectEditor *project = new ProjectEditor(projectPath, projist);
    stackedWidget->setCurrentIndex( stackedWidget->addWidget( project) );
    mapproj.insert(projectPath,project);
}

void StartPage::displayCode(const QString &codePath)
{
    if( codePath.isEmpty() )
    {
        QWidget *widget = stackedWidget->currentWidget();
        assert(widget);
        const QString path = widget->windowFilePath();
        if(! mapcode.contains(path)) return;

        stackedWidget->setCurrentWidget(welcomeWidget);
        return;
    }

    if(mapcode.contains(codePath))
    {
        stackedWidget->setCurrentWidget(mapcode.value(codePath));
        return;
    }

    CodeEditor *code = new CodeEditor(codePath, codist);
    stackedWidget->setCurrentIndex( stackedWidget->addWidget( code));
    mapcode.insert(codePath,code);
}

bool StartPage::isWindowModified() const
{
    return projist->isModified() || codist->isModified();
}
