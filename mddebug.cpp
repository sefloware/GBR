/** @file mddebug.cpp
* @brief the model debuger.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the model debuger Class.
*/

#include "mddebug.h"
#include "config.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QDir>
#include <QPlainTextEdit>
#include <QByteArray>
#include <QStandardItemModel>
#include <QTextCodec>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QAction>

MdDebug::MdDebug(QWidget *parent) :
    QToolBar(parent)
{
    box = new QComboBox;
    box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    box->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    clearAction = new QAction(QIcon(":/icon/images/clean.png"), tr("Clear"),this);
    settingAction = new QAction(QIcon(":/icon/images/settings.png"),"Setting",this);
    stopAction = new QAction(QIcon(":/icon/images/stop.png"), tr("Stop"),this);
    stopAction->setEnabled(false);
    runAction = new QAction(QIcon(":/icon/images/run.png"), tr("Run"),this );
    exploreAction = new QAction(QIcon(":/icon/images/explore.png"),tr("Explore"),this );

    switcher = new QToolButton;
    switcher->setText( tr("Enable Debugging"));
    switcher->setToolButtonStyle(Qt::ToolButtonTextOnly);
    switcher->setObjectName("Switcher");
    switcher->setCheckable(true);

    textOutput = new QPlainTextEdit;
    textOutput->setObjectName("DebugDisplayer");
    textOutput->setReadOnly(true);

    process = new QProcess(this);

    QLabel *envLabel = new QLabel("Execution Environment: ");
    envLabel->setObjectName("TitleLabel");

    this->setObjectName("TitleBar");
    this->addWidget(envLabel);
    this->addWidget (box);
    this->addAction(settingAction);
    this->addSeparator();
    this->addAction (runAction);
    this->addAction (stopAction);
    this->addSeparator();
    this->addAction (exploreAction);
    this->addSeparator();
    this->addAction (clearAction);

    QWidget *sep = new QWidget;
    sep->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->addWidget(sep);
    this->addWidget(switcher);

    setDebugMode(false);
    clearOutputWindow();

    box->setModel(&runevmodel());
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    connect(process, SIGNAL(readyRead()), this, SLOT(readOutput()));
    connect(process, SIGNAL(error(QProcess::ProcessError)),this,SLOT(handError(QProcess::ProcessError)) );
    connect(process,SIGNAL(finished(int)),this,SLOT(slotFinished(int)) );

    cmdformat.setForeground(Qt::blue);
    outformat.setForeground(Qt::darkGreen);
    exitformat.setForeground(Qt::darkMagenta);
    errformat.setForeground(Qt::magenta);

    connect(runAction,SIGNAL(triggered()),this,SLOT(runProcess()) );
    connect(stopAction,SIGNAL(triggered()),this,SLOT(stopProcess()) );

    connect(switcher,SIGNAL(clicked(bool)),this,SLOT(setDebugMode(bool)) );
    connect(clearAction,SIGNAL(triggered()),this,SLOT(clearOutputWindow()) );
    connect(exploreAction,SIGNAL(triggered()),this,SIGNAL(exploreDebugFolder()) );
    connect(settingAction,SIGNAL(triggered()),this,SLOT(editRunEnv()) );
}

MdDebug::~MdDebug()
{
    if(stopAction->isEnabled())
        stopProcess();
}

void MdDebug::stopProcess()
{
    process->kill();
    runAction->setEnabled(true);
}

void MdDebug::runProcess()
{
    runAction->setEnabled(false);
    stopAction->setEnabled(true);
    emit runTriggered();
}

void MdDebug::clearOutputWindow()
{
    textOutput->clear();
    QTextCharFormat format;
    format.setForeground(Qt::black);
    format.setFontWeight(QFont::Bold);
    format.setFontPointSize(16);
    textOutput->setCurrentCharFormat(format);
    textOutput->setPlainText("Output Window");
}

void MdDebug::editRunEnv()
{ editrunenv(this); }

void MdDebug::setWorkingDirectory(const QString &dir)
{ process->setWorkingDirectory(dir); }

void MdDebug::setReadOnly(bool ro)
{  runAction->setEnabled(process->atEnd() && !ro); }

void MdDebug::run(const QStringList &commands)
{
    this->commands = commands;
    if( ! this->commands.isEmpty())
    {
        const QString text = this->commands.takeFirst();
        textOutput->setCurrentCharFormat(cmdformat);
        textOutput->appendPlainText("> "+text);
        process->start(text);
    }
}

QModelIndex MdDebug::currentIndex() const
{  return runevmodel().index(box->currentIndex(),0); }

QDir MdDebug::workingDirectory() const
{ return QDir(process->workingDirectory()); }

void MdDebug::readOutput()
{
    QByteArray text = process->readAll();
    if(text.isEmpty())
        return;
    QTextCodec *codec = QTextCodec::codecForLocale();
    textOutput->setCurrentCharFormat(outformat);
    textOutput->appendPlainText(codec->toUnicode(text));

}

void MdDebug::slotFinished(int exitCode)
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    textOutput->setCurrentCharFormat(exitformat);
    textOutput->appendPlainText(codec->toUnicode(process->readAllStandardError()) );

    if(this->commands.isEmpty())
    {
        textOutput->setCurrentCharFormat(exitformat);
        textOutput->appendPlainText("\n---end---\n");
        runAction->setEnabled(true);
        stopAction->setEnabled(false);
        return;
    }

    if( ! exitCode )
    {
        const QString text = this->commands.takeFirst();
        textOutput->setCurrentCharFormat(cmdformat);
        textOutput->appendPlainText("> "+text);
        process->start(text);
    }
    else
    {
        runAction->setEnabled(true);
        stopAction->setEnabled(false);
    }
}

void MdDebug::setDebugMode(bool show)
{
    QList<QAction *> actionList = this->actions();
    for (int i=0; i< actionList.count()-2; ++i)
        actionList[i]->setVisible(show);

    textOutput->setVisible(show);
    switcher->setText( show ? tr("Disable Debugging") : tr("Enable Debugging"));
}

void MdDebug::handError(QProcess::ProcessError error)
{
    textOutput->setCurrentCharFormat(errformat);
    switch (error) {
    case QProcess::FailedToStart :
        textOutput->appendPlainText(trUtf8("The process failed to start. Either the invoked program is missing, or you may have insufficient  permissions to invoke the program."));
        break;
    case QProcess::Crashed :
        textOutput->appendPlainText(trUtf8("The process crashed some time after starting successfully."));
        break;
    case QProcess::ReadError:
        textOutput->appendPlainText(trUtf8("An error occurred when attempting to read from the process. For example, the process may not be running."));
        break;
    default:
        textOutput->appendPlainText(trUtf8("An unknown error occurred. "));
        break;
    }
}
