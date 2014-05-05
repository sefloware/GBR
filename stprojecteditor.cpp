/** @file stprojecteditor.cpp
* @brief the project description editor in the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the project description editor which
* is composed of the edit bar and the RichTextWidget object.
*/

#include "stprojecteditor.h"
#include "richtextwidget.h"
#include "stproject.h"
#include "config.h"
#include <QStackedLayout>
#include <QToolTip>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QToolBar>
#include <QLabel>

ProjectEditor::ProjectEditor(const QString &projectpath, Projist *projist, QWidget *parent) :
    QWidget(parent),projist(projist)
{
    QWidget::setWindowFilePath(projectpath);

    labelTitle = new QLabel;
    labelTitle->setObjectName("TitleLabel");
    switcher = new QToolButton;
    switcher->setText(tr("Enable Editing"));
    switcher->setToolButtonStyle(Qt::ToolButtonTextOnly);
    switcher->setObjectName("Switcher");
    switcher->setCheckable(true);

    bar = new QToolBar;
    bar->setObjectName("TitleBar");
    bar->addWidget(labelTitle);
    QWidget *seperator = new QWidget;
    seperator->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    bar->addWidget(seperator);
    bar->addWidget(switcher);

    richTextWidget = new RichTextWidget;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(bar);
    layout->addWidget(richTextWidget);

    const QString descriptionPath = descriptionFilePath(projectpath);
    if(! richTextWidget->fileOpen(descriptionPath) )
    {
        QMessageBox::warning(this,tr("Warning"), "Failed to open file:\n" + descriptionPath, QMessageBox::Ok, QMessageBox::Ok);
        this->setEnabled(false);
        return;
    }

    connect(switcher,SIGNAL(toggled(bool)),this,SLOT(setReadWrite(bool)) );
    connect(richTextWidget,SIGNAL(modificationChanged(bool)),this,SLOT(setModification(bool)) );

    setModification(false);
    setReadWrite(false);
}

QString ProjectEditor::descriptionFilePath(const QString &projectPath)
{
    QFileInfo info (projectPath);
    if(! info.isNativePath() || ! info.isDir() )
        return QString();

    const QDir dir(info.absoluteFilePath());

    QString fn;

    fn = info.fileName()+".html";
    if(dir.exists(fn))
        return dir.absoluteFilePath(fn);

    fn = info.fileName()+".htm";
    if(dir.exists(fn))
        return dir.absoluteFilePath(fn);

    QStringList nameFilter;
    nameFilter << "*.html" << "*.htm";

    QStringList fileNames = dir.entryList(nameFilter,QDir::Files,QDir::Time).filter(QRegExp("^\\D.*"));

    if(! fileNames.isEmpty())
        return dir.absoluteFilePath(fileNames.first());

    fn = info.fileName()+".html";
    return dir.absoluteFilePath(fn);
}

void ProjectEditor::setWindowFilePath(const QString &projectPath)
{
    QWidget::setWindowFilePath(projectPath);
    richTextWidget->setWindowFilePath(projectPath);

    QString title = this->windowTitle();
    title.replace("[*]",isWindowModified() ? "*" : "");
    labelTitle->setText(title);
}

bool ProjectEditor::save()
{
    return richTextWidget->fileSave();
}

void ProjectEditor::setReadWrite(bool rw)
{
    if(! rw && (isWindowModified() && ! richTextWidget->maybeSave()) )
    {
        QToolTip::showText(QCursor::pos(),"Failed to disable editing!");
        switcher->setChecked(true);
        return;
    }

    switcher->setText(rw ? "Disable Editing" : "Enable Editing");
    richTextWidget->setReadOnly(!rw);
}


void ProjectEditor::setModification(bool modified)
{
    setWindowModified(modified);

    QString title = this->windowTitle();
    title.replace("[*]",modified ? "*" : "");
    labelTitle->setText(title);

    projist->setModification(this->windowFilePath(),modified);
}
