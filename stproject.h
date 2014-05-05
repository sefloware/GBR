/** @file stproject.h
* @brief the project area  in the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the project area which is
* composed of the title, the bar, the data model
* for the project sets and the view.
*/

#ifndef STPROJECT_H
#define STPROJECT_H

#include <QFileSystemModel>
#include <QTreeView>

QT_BEGIN_NAMESPACE
class QToolBar;
class QTreeView;
class QDir;
QT_END_NAMESPACE

class ProjectModel :public QFileSystemModel
{
    Q_OBJECT
public:
    explicit ProjectModel(const QSet<QString> &openedProjectSet,const QSet<QString> &modifiedProjectSet,QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
private:
    const QSet<QString> &modifiedProjectSet;
    const QSet<QString> &openedProjectSet;

    QIcon openedIcon;
    QIcon closedIcon;
};

class Projist : public QWidget
{
    Q_OBJECT
    
public:
    explicit Projist(QWidget *parent = 0);
    void removeMark(const QString &path);
    void clearSelection();
    bool isModified()const;
public slots:
    void setModification(const QString &project, bool modified);
signals:
    void aboutToOpenPath(const QString &path);
    void updateResultTable(const QString &path);
    void currentChanged(const QString &path);
private slots:
    void slotFileRenamed(const QString &path, const QString &oldName, const QString &newName);
    void emitProjectPath(const QModelIndex &index);
    void importSimulations();
    void resizeViewWidth();
    void openProject();
    void updateAction(const QModelIndex &current);
    void removeProject();
    void newProject();
    void explorCurrent();
    void derive();
    void rename();
private:
    bool importFile(const QString &source, const QDir &destination);
    bool importStfile(const std::ifstream &source, std::ofstream &destination);
private:
    QAction *newAction;
    QAction *openAction;
    QAction *removeAction;
    QAction *importAction;
    QAction *explorAction;

    QAction *renameAction;
    QAction *deriveAction;

    QToolBar *bar;
public:
    QTreeView *view;
    ProjectModel *model;
    QSet<QString> openedProjectSet;
    QSet<QString> modifiedProjectSet;
};

#endif // STPROJECT_H
