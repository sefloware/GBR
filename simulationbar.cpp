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

SimulationBar::SimulationBar(QWidget *parent) :
    QToolBar(parent)
{ 
    envModel = new QFileSystemModel(this);
    envModel->setFilter(QDir::Files);
    envModel->setNameFilters(QStringList()<< "*.cmd" << "*.sh");
    envModel->setRootPath(absolutePath(ConfigFolder));

    box = new QComboBox;
    box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    box->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    box->setModel(envModel);

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

    QLabel *envLabel = new QLabel("Execution Environment: ");
    envLabel->setObjectName("TitleLabel");

    QWidget *sep = new QWidget;
    sep->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    this->setObjectName("TitleBar");
    this->addWidget(envLabel);
    this->addWidget (box);
    this->addWidget (exportButton);
    this->addWidget(sep);
    this->addWidget(simuButton);

    showSimulation(false);

    connect(exportButton,SIGNAL(clicked()),this,SIGNAL(exportTriggered()) );
    connect(simuButton,SIGNAL(toggled(bool)),this,SLOT(showSimulation(bool)) );
}

QFileInfo SimulationBar::currentEnvInfo() const
{  return QFileInfo(envModel->rootDirectory().absoluteFilePath(box->currentText()) );}

void SimulationBar::showSimulation(bool show)
{
    QList<QAction *> listActions = this->actions();
    for (int i=0; i<listActions.count()-2; ++i)
        listActions[i]->setVisible(show);

    simuButton->setText( show ? tr("Disable Simulating") : tr("Enable Simulating"));
    emit simulationTriggered(show);
}
