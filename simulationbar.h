/** @file simulationbar.h
* @brief the simulation bar.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the simulation bar for
* exporting computing task.
*/

#ifndef SIMULATIONBAR_H
#define SIMULATIONBAR_H

#include <QToolBar>

QT_BEGIN_NAMESPACE
class QComboBox;
class QToolButton;
class QLabel;
class QFileSystemModel;
class QFileInfo;
QT_END_NAMESPACE

class SimulationBar : public QToolBar
{
    Q_OBJECT
public:
    SimulationBar(QWidget *parent = 0);
    QFileInfo currentEnvInfo() const;
signals:
    void exportTriggered();
    void simulationTriggered(bool show);
private slots:
    void showSimulation(bool show);
    void exploreRunScripts();
private:
    QToolButton *exportButton;
    QToolButton *simuButton;
    QToolButton *envButton;
    QComboBox *box;

    QFileSystemModel *envModel;
};

#endif // SIMULATIONBAR_H
