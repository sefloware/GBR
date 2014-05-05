/** @file stcode.h
* @brief the source code area  in the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the source code area which
* is composed of the title, the bar, the data model
* for the souce code resource and the view.
*/

#ifndef STCODE_H
#define STCODE_H

#include <QFileSystemModel>
#include <QTreeView>

QT_BEGIN_NAMESPACE
class QToolBar;
class QTreeView;
class QDir;
QT_END_NAMESPACE

class CodeModel :public QFileSystemModel
{
    Q_OBJECT
public:
    explicit CodeModel(const QSet<QString> &modifiedSourceSet, QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
private:
    const QSet<QString> &modifiedSourceSet;
    QIcon icon;
};

class Codist : public QWidget
{
    Q_OBJECT
public:
    explicit Codist( QWidget *parent = 0);

    void clearSelection();
    bool isModified() const;

    static bool copyto(const QString &path);
    static bool recurseCopyDir(QDir &s,QDir &d);
signals:
    void currentChanged(const QString &path);
    void updateEditorEnvironment();
public slots:
    void setModification(const QString &file, bool modified);
private slots:
    void slotFileRenamed(const QString &path, const QString &oldName, const QString &newName);

    void emitCodePath(const QModelIndex &current);

    void newSource();
    bool removeSource();
    void resizeViewWidth();
    void updateAction(const QModelIndex &current);
    void explorCurrent();

    void rename();
private:
    void registerAllToSource();
    void registerToSource(const QFileInfo &fileInfo);
private:
    QToolBar *bar;
    QTreeView *view;
    CodeModel *model;

    QAction *actionDelete;
    QAction *newAction;
    QAction *explorAction;
    QAction *renameAction;

    QSet<QString> modifiedSourceSet;

    friend class StartPage;
};

#endif // STCODE_H
