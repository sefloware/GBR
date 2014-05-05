/** @file pjanalyser.h
* @brief the data-analysing interface of one project.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the data analyser Class, which is
* used for analyse simulation datas and export computing
* tasks.
*/

#ifndef PJANALYSER_H
#define PJANALYSER_H

#include <QWidget>
#include <QDir>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
class QLabel;
class QToolBar;
class QSqlTableModel;
class QStandardItem;
class QSqlDatabase;
class QToolButton;
QT_END_NAMESPACE

class TableFilter;
class ParoutModel;
class TableView;
class SimulationBar;
class LineEdit;

class PjAnalyser : public QWidget
{
    Q_OBJECT
    
public:
    explicit PjAnalyser(const QDir &root,
                       QWidget *parent = 0 );

    void setSourceModel(ParoutModel *sourceModel);
    void setReadOnly(bool ro);
signals:
    void switch2Model();
private slots:
    void exportSimulationTask();
    void setCurrentHorizontalHeader(const QString &header);
    void setLineEdit(const QModelIndex &current);
private:
    bool lockParameter();
private:
    const QDir &root;
    ParoutModel *pm;

    QLabel *label;
    LineEdit *lineEdit;
    QToolBar *toolbar;   
    QAction *refreshAction;
    QToolButton *previous;
    SimulationBar *simExport;
public:
    TableFilter *tableFilter;
    TableView *tableView;
};

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget *parent = 0) : QLineEdit(parent)
    { connect(this,SIGNAL(editingFinished()),this,SLOT(finishEditing()) ); }
signals:
    void editingFinished(const QString &str);
public slots:
    void setText(const QString &str,bool readonly )
    {
        QLineEdit::setText(str);
        setReadOnly(readonly);
    }
private slots:
    void finishEditing()
    { emit editingFinished(text()); }
};

#endif // PJANALYSER_H
