/** @file pjmodeler.h
* @brief the modeling interface of one project.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the modeler Class, which is
* used for modeling.
*/

#ifndef PJMODELER_H
#define PJMODELER_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QModelIndexList>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QSqlDatabase;
class QToolButton;
class QListView;
class QItemSelection;
class QToolBar;
class QLabel;
class QDir;
QT_END_NAMESPACE
class ParoutView;
class ParoutModel;
class MainEditor;
class MdDebug;
class ParoutDelegate;

class PjModeler : public QWidget
{
    Q_OBJECT
    
public:
    explicit PjModeler(const QDir &dir, QWidget *parent, QSqlDatabase db);

    void saveTemporary();
public slots:
    bool save();
private slots:
    void debug();
    void exploreDebugFolder();
    bool newChain();
    bool newOutput();

    void readRegion();
    void deleteExtraSelections();
    void highlightCurrentSelection();
    void regionViewerSelectionChanged();
    void setReadWrite(bool rw);
    void setReadOnlyMode(bool ro);
    void slotModificationChanged(bool modified);
private:
    QList<QTextEdit::ExtraSelection> transformToExtraSelection(const QList<QTextCursor> &tcs);
    QList<QTextEdit::ExtraSelection> transformToExtraSelection(QTextCursor tc);
    QTextEdit::ExtraSelection extraSelection(const QTextCursor &tc);
    void highlightIndexs(const QModelIndexList &indexs);
    bool containRegion(QString secName) const;
signals:
    void switch2Analyse();
    void modificationChanged(bool changed);
private:
    const QDir &dir;

    QToolBar *titleBar;

    QLabel *titleLabel;
    QAction *deleteAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *saveAction;
    QToolButton *addchainButton;
    QToolButton *addoutputButton;
    QToolButton *editButton;
    QToolButton *nextButton;

    QAction *editAction;

    MainEditor *editor;
    QListView *regionViewer;
    QStandardItemModel *regionModel;
    ParoutView *paroutViewer;
    ParoutDelegate *paroutDelegate;
    MdDebug *mdDebug;
    ParoutModel *paroutModel;

    friend class Project;
};

#endif // PJMODELER_H
