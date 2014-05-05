/** @file mdparout.h
* @brief the viewer for parameters and output files.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the model and the view of
* parameters and output files.
*/

#ifndef MDPAROUT_H
#define MDPAROUT_H

#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QTreeView>
#include <QHeaderView>
#include <QAction>

QT_BEGIN_NAMESPACE
class QSqlTableModel;
class QDir;
QT_END_NAMESPACE

class MainEditor;
class TableResult;

class ParoutModel : public QStandardItemModel
{
    Q_OBJECT
public:
    ParoutModel(const QDir &simdatadir,MainEditor *parent, QSqlDatabase db = QSqlDatabase());

    bool initialize();
    bool reselect();

    QSqlTableModel *parameterModel() const
    { return tablePara; }
    QSqlDatabase database() const
    { return db; }
    TableResult *resultModel() const
    { return tableResult; }
    QStringList outputs() const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);    
    bool generateMain(const QString &mainsource,const QString &path) const;

    bool saveHeaders();
    bool saveParout();

    QStandardItem *newLastRowItem() const;
    QStandardItem *newKeyItem(const QVariant &key) const;
    QStandardItem *newFileformulaItem(const QString &text) const;
    static void tranformToParafilterItem(QStandardItem *paramterItem,const QVariant &value);
private:
    QStandardItem *cloneParoutItem(const QString &text) const;
    QStandardItem *newFormulaItem(const QString &formula) const;
    QStandardItem *newTimeItem() const;
    QStandardItem *newIdItem() const;
signals:
    void addFilterAndHideColumn(int section, const QVariant &value);
    void deleteFilterAndShowColumn(int section);
    void initializeResultViewApperance();
    void simulationExistence(bool);
private:
    QMap<QString,QVariant> selectParoutMap(const QSqlDatabase &db) const;
    QMap<QString, QVariant> selectParoutMap() const;
    bool generateParaSQL(const QSqlDatabase &db) const;
    void readParout(const QMap<QString, QVariant> &map);
    bool generateResultTable();

    bool paratableVsparoutable() const;
    bool insureParatableValid();
public:
    QString keyPath(const QString &key) const;
    QString filePath(const QString &key, const QString &name) const;
    QStringList filePathList(const QString &key, const QString &name) const;
    std::vector<double> readStFile(const QString &key, const QString &stFile) const;
    int stRecordCount(const QString &key, const QString &stFile) const;
    int dyRecordCount(const QString &key, QString fileName) const;
    bool removeFile(const QString &key, const QString &fileName) const;
    bool removeKey(const QString &key) const;
    bool removeSimulation() const;
    bool existSimulation(const QString &key) const;
    void checkNewoutData(const QStringList &newout) const;
private:
    MainEditor *editor;
    QSqlDatabase db;
    const QDir &simdatadir;
    int paracount;
    QSqlTableModel *tablePara;
    TableResult *tableResult;
};

class ParoutView : public QTreeView
{
    Q_OBJECT
public:
    explicit ParoutView(QWidget *parent = 0) :
        QTreeView(parent)
    {
        clearAction = new QAction(tr("Clear"), this);
        clearAction->setShortcut(Qt::Key_Delete);
        connect(clearAction, SIGNAL(triggered()), this, SLOT(clear()));
        addAction(clearAction);

        connect(header(),SIGNAL(sectionResized(int,int,int)),this,SLOT(resizeToContent(int,int,int)) );
    }
    void resizeToContent()
    { this->setFixedWidth(columnWidth(0) + columnWidth(1)); }
public slots:
    void resizeToContent(int logicalIndex, int /*oldSize*/, int newSize) {
        newSize += (logicalIndex ? columnWidth(0) : columnWidth(1) );
        setFixedWidth(newSize);
    }
private slots:
    void clear() {
       QModelIndexList idxs = this->selectedIndexes();
       foreach (const QModelIndex &it , idxs) {
           if(it.column() == 1)
               model()->setData(it,QVariant());
       }
    }
private:
    QAction *clearAction;
};

#endif // MDPAROUT_H
