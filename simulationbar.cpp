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

SimulationBar::SimulationBar(QWidget *parent) :
    QToolBar(parent)
{
    box = new QComboBox;
    box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    box->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    box->setModel(&runevmodel());

    settingAction = new QAction(QIcon(":/icon/images/settings.png"), tr("Setting..."),this);

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
    this->addAction(settingAction);
    this->addSeparator();
    this->addWidget (exportButton);
    this->addWidget(sep);
    this->addWidget(simuButton);

    showSimulation(false);

    connect(settingAction,SIGNAL(triggered(bool)),this,SLOT(editRunEnv()) );
    connect(exportButton,SIGNAL(clicked()),this,SIGNAL(exportTriggered()) );
    connect(simuButton,SIGNAL(toggled(bool)),this,SLOT(showSimulation(bool)) );
}

void SimulationBar::editRunEnv()
{ editrunenv(this); }

QModelIndex SimulationBar::currentIndex() const
{  return runevmodel().index(box->currentIndex(),0); }

void SimulationBar::showSimulation(bool show)
{
    QList<QAction *> listActions = this->actions();
    for (int i=0; i<listActions.count()-2; ++i)
        listActions[i]->setVisible(show);

    simuButton->setText( show ? tr("Disable Simulating") : tr("Enable Simulating"));
    emit simulationTriggered(show);
}
