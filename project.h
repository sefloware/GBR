/** @file project.h
* @brief the project widget.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the project widget composed of
* modeler and analyser widget.
*/

#ifndef PROJECT_H
#define PROJECT_H

#include <QStackedWidget>
#include <QSqlDatabase>
#include <QDir>

QT_BEGIN_NAMESPACE
class QTabWidget;
QT_END_NAMESPACE
class PjModeler;
class PjAnalyser;
class MainWindow;

class Project : public QStackedWidget
{
    Q_OBJECT
public:
    explicit Project(const QString &path, QTabWidget *parent=0);
    ~Project();
    void refresh();
    void updateEditorEnvironment();
private slots:
    void switch2Model();
    void switch2Analyse();
    void slotModificationChanged(bool modified);
public:
    const QDir dir;
    QTabWidget *parentTab;

    PjModeler *pjmd;
    PjAnalyser *pjal;
};

#endif // PROJECT_H
