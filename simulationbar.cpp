/** @file simulationbar.cpp
* @brief the simulation bar.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the simulation bar for
* exporting computing task.
*/

#include "simulationbar.h"
#include "config.h"
#include <QComboBox>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QUrl>
#include <QDesktopServices>
#include <QListView>
#include <QDebug>

SimulationBar::SimulationBar(QWidget *parent) :
    QToolBar(parent)
{ 
    envModel = new QFileSystemModel(this);
    envModel->setNameFilterDisables(false);
    envModel->setNameFilters(QStringList()<< "*.run.cmd" << "*.run.sh");
    envModel->setFilter(QDir::Files);

    box = new QComboBox;
    box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    box->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QString tmpPath = absolutePath(ConfigFolder);
    if( !tmpPath.isEmpty())
    {
        QModelIndex tmpIndex = envModel->setRootPath(tmpPath);
        box->setModel(envModel);
        box->setRootModelIndex(tmpIndex);
        box->setCurrentIndex(0);
    }

    exportButton = new QToolButton;
    exportButton->setIcon(QIcon(":/icon/images/task.png"));
    exportButton->setText(tr("Export Tasks"));
    exportButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    exportButton->setObjectName("ToolButton");

    simuButton = new QToolButton;
    simuButton->setText(tr("Enable Simulating"));
    simuButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    simuButton->setCheckable(true);
    simuButton->setObjectName("Switcher");

    envButton = new QToolButton(this);
    envButton->setText("Execution Environment");
    envButton->setObjectName("TitleButton");

    QWidget *sep = new QWidget;
    sep->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    this->setObjectName("TitleBar");
    this->addWidget(envButton);
    this->addWidget (box);
    this->addWidget (exportButton);
    this->addWidget(sep);
    this->addWidget(simuButton);

    showSimulation(false);

    connect(exportButton,SIGNAL(clicked()),this,SIGNAL(exportTriggered()) );
    connect(simuButton,SIGNAL(toggled(bool)),this,SLOT(showSimulation(bool)) );
    connect(envButton,SIGNAL(clicked()), this, SLOT(exploreRunScripts()));
}

QFileInfo SimulationBar::currentEnvInfo() const
{  return QFileInfo(envModel->rootDirectory().absoluteFilePath(box->currentText()) );}

void SimulationBar::exploreRunScripts()
{
    QString tmp = absolutePath(ConfigFolder);
    if( !tmp.isEmpty())
        QDesktopServices::openUrl( QUrl::fromLocalFile(tmp));
}

void SimulationBar::showSimulation(bool show)
{
    QList<QAction *> listActions = this->actions();
    for (int i=0; i<listActions.count()-2; ++i)
        listActions[i]->setVisible(show);

    simuButton->setText( show ? tr("Disable Simulating") : tr("Enable Simulating"));
    emit simulationTriggered(show);
}
