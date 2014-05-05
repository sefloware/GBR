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
QT_END_NAMESPACE

class SimulationBar : public QToolBar
{
    Q_OBJECT
public:
    SimulationBar(QWidget *parent = 0);
    QModelIndex currentIndex() const;
signals:
    void exportTriggered();
    void simulationTriggered(bool show);
private slots:
    void showSimulation(bool show);
    void editRunEnv();
private:
    QAction *settingAction;
    QToolButton *exportButton;
    QToolButton *simuButton;
    QComboBox *box;
};

#endif // SIMULATIONBAR_H
