/** @file stcodeeditor.h
* @brief the source code editor in the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the source code editor which
* is composed of the edit bar, the Editor2 object,
* the C++ objects model and view.
*/

#ifndef STCODEEDITOR_H
#define STCODEEDITOR_H

#include "startpage.h"
#include <map>

QT_BEGIN_NAMESPACE
class QTreeView;
class QToolBar;
class QLabel;
class QToolButton;
class QStandardItem;
class QStandardItemModel;
QT_END_NAMESPACE
class Editor2;
class Codist;

namespace client
{
class dimeninfo;

typedef std::pair<std::string, std::string> range;
typedef std::map<dimeninfo, std::vector<range> > dimenmap;

typedef std::vector<std::string> termtype;
typedef std::vector<termtype> expressiontype;
}

class CodeEditor : public QWidget
{
    Q_OBJECT
public:
    explicit CodeEditor(const QString &codePath, Codist *refCodist, QWidget *parent = 0);
    ~CodeEditor();
    void setWindowFilePath(const QString &path);
    void updateEditorEnvironment();
private slots:
    bool save();
    bool generateRoadblockCode();
    void setReadWrite(bool rw);
    void setModification(bool modified);
private:
    bool maybeSave();
    bool parsePlane(const QString &record, client::dimenmap &v);
    bool parseAssemble(const QString &assemble, client::expressiontype &exp);
    bool load(const QString &path);
    void select(QStandardItem *parent,const QStandardItem *sourceParent);
private:
    QAction *redoAction;
    QAction *undoAction;
    QAction *saveAction;
    QAction *generateAction;
    QToolButton *switcher;
    QLabel *labelTitle;

    QAction *switchAction;
    QToolBar *titleBar;
    Editor2 *edit;
    QTreeView *view;
    QStandardItemModel *model;

    Codist *refCodist;
};

#endif // STCODEEDITOR_H
