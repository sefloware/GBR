/** @file mdparout.cpp
* @brief the viewer for parameters and output files.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the model and the view of
* parameters and output files.
*/

#include "mdparout.h"
#include "mdeditor.h"
#include "table.h"
#include "item.h"
#include <QSqlTableModel>
#include <QDesktopServices>
#include <QSqlQuery>
#include <fstream>
#include <QMessageBox>
#include <limits>
#include <QToolTip>

ParoutModel::ParoutModel(const QDir &dir, MainEditor *parent, QSqlDatabase db)
    : QStandardItemModel(parent), editor(parent), db(db), simdatadir(dir), paracount(0)
{
    tablePara = new QSqlTableModel(this,db);
    tablePara -> setEditStrategy(QSqlTableModel::OnFieldChange);
    tableResult = new TableResult(this,editor);

    QStringList labels;
    labels << "Name" << "Value";
    this->setHorizontalHeaderLabels(labels);
}

bool ParoutModel::saveHeaders()
{
    assert( db.isValid() );
    QSqlQuery query(db);
    if( !query.exec("drop table if exists header") || ! query.exec("create table header(section integer,name varchar(20))") )
        return false;

    query.prepare("INSERT INTO header (section, name) "
                  "VALUES (?, ?)");

    for (int section = tableResult->parameterColumnCount()+1; section < tableResult->columnCount(); ++section)
    {
        QString text = tableResult->headerText(section);

        query.addBindValue(section);
        query.addBindValue(text);

        if(! query.exec() )
            return false;
    }
    return true;
}

bool ParoutModel::saveParout()
{
    assert( db.isValid() );
    QSqlQuery query(db);
    if( !query.exec("drop table if exists parout") || ! query.exec("create table parout(name varchar(20),value)") )
        return false;

    query.prepare("INSERT INTO parout (name, value) "
                  "VALUES (?, ?)");

    for (int row = 0; row < paracount; ++row)
    {
        QVariant name = data(index(row,0));
        assert( name.isValid());

        QVariant value = data(index(row,1));
        if(value.toString() == QLatin1String("?"))
            continue;

        query.addBindValue(name);
        query.addBindValue(value);
        if(! query.exec() )
            return false;
    }
    return true;
}

QMap<QString,QVariant> ParoutModel::selectParoutMap() const
{
    QMap<QString,QVariant>  result;
    for(int row=0; row<paracount; ++row)
    {        
        QString name = data(index(row,0)).toString();
        assert(! name.isEmpty());

        QVariant value = data(index(row,1));
        if(value.toString() == QLatin1String("?"))
            continue;

        result.insert(name,value);
    }
    return result;
}

QMap<QString, QVariant> ParoutModel::selectParoutMap(const QSqlDatabase &db) const
{
    QMap<QString,QVariant>  result;

    QSqlQuery query("select * from parout",db);
    while( query.next())
    {
        QString name = query.value(0).toString();
        assert(! name.isEmpty());

        QVariant value = query.value(1);
        if(value.toString() == QLatin1String("=") )
            continue;

        result.insert(name, value);
    }
    return result;
}

bool ParoutModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(! index.isValid())
        return false;

    switch(role)
    {
    case Qt::EditRole:
    {
        if(index.column() != 1)
            return false;

        const QModelIndex hidx = QStandardItemModel::index(index.row(),0);
        QFont font = data(index,Qt::FontRole).value<QFont>();
        if(value.isValid() )
        {
            QStandardItemModel::setData(hidx, value,Item::DataRole);
            font.setUnderline(true);
            QStandardItemModel::setData(hidx,font,Qt::FontRole);
            QStandardItemModel::setData(hidx,Item::Parafiter,Item::HeaderFlagRole);

            const int section = hidx.row() + 1;
            emit addFilterAndHideColumn(section,value);

            return QStandardItemModel::setData(index,value,Qt::EditRole);
        }
        else
        {
            font.setUnderline(false);
            QStandardItemModel::setData(hidx,font,Qt::FontRole);
            QStandardItemModel::setData(hidx, Item::Parameter, Item::HeaderFlagRole);

            const int section = hidx.row() + 1;
            emit deleteFilterAndShowColumn(section);

            return QStandardItemModel::setData(index,"?",Qt::EditRole);
        }
    }
    default:
        return QStandardItemModel::setData(index,value,role);
    }
}

bool ParoutModel::generateParaSQL(const QSqlDatabase &db) const
{
    QSqlQuery query(db);

    // parameters
    QStringList paralist("id integer PRIMARY KEY autoincrement");
    for (int i = 0; i < paracount; ++i)
       paralist << item(i)->text();

    query.exec("drop table if exists parameter");
    if(! removeSimulation())
        return false;
    if(! query.exec("create table parameter(" + paralist.join(',')+")") )
        return false;
    return true;
}

bool ParoutModel::paratableVsparoutable() const
{
    //don't explicitly call this function. See function: insureParatableValid().
    if(tablePara->columnCount() != paracount + 1)
        return false;

    for (int i = 0; i < paracount; ++i)
    {
        QString text = tablePara->headerData(i+1,Qt::Horizontal).toString();
        if(text != item(i)->text())
            return false;
    }
    return true;
}

bool ParoutModel::insureParatableValid()
{
    //don't explicitly call this function. See function: loadTableResult().
    if( paratableVsparoutable() )
        return true;

    if(tablePara->rowCount())
    {
        //Datas of tablePara exists.
        int answer = QMessageBox::critical(editor, tr("Error"),
                                           tr("The old result table exists whose horizontal header is inconsistent with the current parameters"),
                                           QMessageBox::Discard | QMessageBox::Ignore, QMessageBox::Discard);
        if( answer == QMessageBox::Ignore)
            return false;
    }

    if(! generateParaSQL(db))
    {
        QMessageBox::critical(editor, tr("Error"), tr("Failed to generate the result table or to clean the old simulation datas!"),  QMessageBox::Ok , QMessageBox::Ok);
        return false;
    }

    tablePara->setTable("parameter");
    if ( ! tablePara->select() )
    {
        QMessageBox::critical(editor, tr("Error"), tr("Fail to import datas from the database!"), QMessageBox::Ok , QMessageBox::Ok);
        return false;
    }

    return true;
}

bool ParoutModel::generateResultTable()
{
    //important
    tableResult -> clear();

    if( ! insureParatableValid())
        return false;

//  set the horizontal header items.
    tableResult->setHorizontalHeaderItem(0,newIdItem());
    for (int i = 0; i < paracount; ++i)
       tableResult->setHorizontalHeaderItem( i+1, item(i)->clone());
   tableResult->setHorizontalHeaderItem( paracount+1, newTimeItem());

//    set the vertical header items.
    for(int i = 0; i < tablePara->rowCount(); ++i)
        tableResult->setVerticalHeaderItem(i, newKeyItem( tablePara->index(i,0).data()));

    tableResult->setVerticalHeaderItem(tablePara->rowCount(),newLastRowItem());
    return true;
}

void ParoutModel::readParout(const QMap<QString,QVariant> &map)
{
    assert( editor );
    this->setRowCount(0);

    QList<QStandardItem *> list;
    static QStandardItem iq;
    typedef QList<QStandardItem *> QStandardItemList;

    QSet<QString> set;
    set.insert("Id");
    for(int i=0; i<editor->localObjModel()->rowCount(); ++i)
    {
        QStandardItem *item = editor->localObjModel()->item(i);
        if(     item->data(Item::CppTypeRole).toInt() != Item::Variable ||
                set.contains(item->text()) )
            continue;

        if( item->data(Qt::ToolTipRole).toString().contains(QRegExp("=\\s*\\?\\s*$")) )
        {
            // parameter item.           
            QStandardItem *it = item->clone();
            it->setData(Item::Parameter,Item::HeaderFlagRole);
            it->setIcon(Item::headerIconMap().value(Item::Parameter));

            iq.setData("?",Qt::EditRole);
            iq.setForeground(Qt::darkGreen);
            iq.setEditable(true);

            QMap<QString,QVariant>::const_iterator iterator = map.find(it->text());
            if(iterator != map.end() )
            {
                assert( iterator.value() != QLatin1String("?"));

                tranformToParafilterItem(it, iterator.value());
                iq.setData( iterator.value(),Qt::EditRole);
            }

            this->appendRow(QStandardItemList() << it << iq.clone());

            set.contains(it->text());
        }
        else if(item->data(Item::PurifiedTypeRole).toString() == QLatin1String("std::ofstream"))
        {
            // file item.
            QStandardItem *it = item->clone();
            QString name = it->data(Qt::ToolTipRole).toString().section('"',1,1);

            Item::HeaderFlags flag = name.endsWith(".st") ? Item::FileSt : Item::FileOt;
            it->setText(name);
            it->setForeground(Qt::black);
            it->setData(flag,Item::HeaderFlagRole);
            it->setIcon(Item::headerIconMap().value(flag));
            list << it;

            set.contains(it->text());
        }
    }
    paracount = this->rowCount();

    iq.setText("-");
    iq.setForeground(Qt::gray);
    iq.setEditable(false);
    foreach ( QStandardItem *it, list)
        this->appendRow(QStandardItemList() << it << iq.clone());


}

bool ParoutModel::initialize()
{
    readParout(selectParoutMap(db));
    tablePara->setTable("parameter");
    tablePara->select();
    if(! generateResultTable() )
        return false;

    QSqlQuery query("select * from header",db);
    while( query.next())
        tableResult->setHeaderText(query.value(0).toInt(),query.value(1).toString() );

    tableResult->calculateAll();

    emit simulationExistence( tablePara->rowCount() );

    return true;
}

bool ParoutModel::reselect()
{
    QStringList oldout;
    for( int i = paracount; i< rowCount(); ++i)
        oldout << data(index(i,0)).toString();

    readParout(selectParoutMap());

    QStringList newout;
    for( int i = paracount; i< rowCount(); ++i)
    {
        const QString text = data(index(i,0)).toString();
        if(! text.isEmpty() && ! oldout.contains(text))
            newout << text;
    }

    checkNewoutData(newout);

    if(! generateResultTable())
        return false;

    for (int i = paracount; i < rowCount(); ++i)
        tableResult->setHorizontalHeaderItem(i+2, item(i)->clone() );

    tableResult->calculateAll();

    emit initializeResultViewApperance();
    emit simulationExistence( tablePara->rowCount() );

    return true;
}

QStandardItem *ParoutModel::newFileformulaItem(const QString &text) const
{
    assert(! text.isEmpty() && text.trimmed() == text);

    QStandardItem  *item = cloneParoutItem(text);
    if( ! item)     item = newFormulaItem(text);
    return item;
}

void ParoutModel::tranformToParafilterItem(QStandardItem *paramterItem, const QVariant &value)
{
    assert(value.isValid());
    QFont font = paramterItem->font();
    paramterItem->setData(value,Item::DataRole);
    font.setUnderline(true);
    paramterItem->setFont(font);
    paramterItem->setData(Item::Parafiter,Item::HeaderFlagRole);
}

QStringList ParoutModel::outputs() const
{
    QStringList result;
    for (int i=paracount; i< rowCount(); ++i)
    {
        QString val = index(i,0).data().toString();
        if(val.isEmpty())
            return QStringList();
        result << val;
    }
    return result;
}

QStandardItem *ParoutModel::newKeyItem(const QVariant &key) const
{
    assert( key.canConvert<int>());

    bool exist = existSimulation(key.toString());
    QStandardItem *item = new QStandardItem;
    item->setData(key,Qt::EditRole);
    item->setData( exist ? Item::SimData : Item::NoSimData, Item::DataRole);
    QFont font = item->font();
    if(exist) font.setItalic(true);
    else font.setItalic(false);
    item->setFont(font);
    item->setData(tr("Key"),Qt::ToolTipRole);
    return item;
}

QStandardItem *ParoutModel::cloneParoutItem(const QString &text) const
{
    for (int i = 0; i < rowCount(); ++i)
        if(text.toUpper() == item(i)->text().toUpper() )
            return item(i)->clone();
    return 0;
}

QStandardItem *ParoutModel::newFormulaItem(const QString &formula) const
{
    QRegExp fm("^([a-zA-Z_]\\w*)\\s*=");
    if(! formula.contains(fm))
        return 0;

    QStandardItem *item = new QStandardItem;
    item->setText(fm.cap(1));
    item->setData(formula.mid(fm.matchedLength()-1),Item::DataRole);
    item->setData(formula,Qt::ToolTipRole);
    item->setData(Item::Formula,Item::HeaderFlagRole);
    item->setIcon(Item::headerIconMap().value(Item::Formula));
    return item;
}

QStandardItem *ParoutModel::newTimeItem() const
{
    QStandardItem *item = new QStandardItem;
    item->setText("<Times>");
    item->setData(tr("Times of Simulations"),Qt::ToolTipRole);
    item->setData(Item::Times,Item::HeaderFlagRole);
    item->setBackground(Qt::lightGray);
    item->setForeground(Qt::blue);
    return item;
}

QStandardItem *ParoutModel::newIdItem() const
{
    QStandardItem *item = new QStandardItem;
    item->setText("ID");
    item->setData(tr("ID of Record"),Qt::ToolTipRole);
    item->setData(Item::Id,Item::HeaderFlagRole);
    item->setIcon(Item::headerIconMap().value(Item::Id));
    return item;
}

QStandardItem *ParoutModel::newLastRowItem() const
{
    QStandardItem *item = new QStandardItem;
    item->setText("*");
    item->setData(tr("Row for Newing Record"),Qt::ToolTipRole);
    item->setData(Item::OtherRow,Item::DataRole);
    item->setBackground(Qt::lightGray);
    return item;
}

int ParoutModel::stRecordCount(const QString &key, const QString &stFile) const
{
    QString filepath = simdatadir.absoluteFilePath(key+QDir::separator()+stFile);
    QByteArray temp = filepath.toLocal8Bit();
    std::ifstream in(temp.data());
    int lineCount =0;
    while(in.good())
    {
        in.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        ++lineCount;
    }
    if(lineCount) return lineCount-1;
    else return 0;
}

int ParoutModel::dyRecordCount(const QString &key, QString fileName) const
{ return filePathList(key,fileName).count(); }

bool ParoutModel::removeKey(const QString &key) const
{ return  QDir(simdatadir.absoluteFilePath(key)).removeRecursively(); }

bool ParoutModel::removeFile(const QString &key, const QString &fileName) const
{
    assert(fileName.count('.') == 1);

    QDir dir(simdatadir.absoluteFilePath(key));
    if(dir.exists(fileName) && fileName.endsWith(".st"))
        return dir.remove(fileName);

    foreach (const QString &it, filePathList(key,fileName)) {
        QFile file(it);
        if(file.exists() && ! QFile(it).remove())
            return false;
    }

    return true;
}

bool ParoutModel::removeSimulation() const
{
    const QStringList list = simdatadir.entryList(QDir::Dirs| QDir::NoDotAndDotDot);
    foreach (const QString &it, list) {
        if(! it.contains(QRegExp("^\\d+$")) )
            continue;
        if(! removeKey(it))
            return false;
    }
    return true;
}

void ParoutModel::checkNewoutData(const QStringList &newout) const
{
    const QStringList list = simdatadir.entryList(QDir::Dirs| QDir::NoDotAndDotDot);

    QStringList pathList;

    std::map<QString,QStringList> msg;
    foreach (const QString &it, list) {
        if(! it.contains(QRegExp("^\\d+$")) )
            continue;
        foreach (const QString &fileName, newout)
        {
            if(fileName.endsWith(".st"))
            {
                const QString path = filePath(it,fileName);
                if(! path.isEmpty())
                {
                    pathList << path;
                    msg[fileName] << it;
                }
            }
            else
            {
                const QStringList paths = filePathList(it,fileName);
                if(! paths.isEmpty())
                {
                    pathList << paths;
                    msg[fileName] << it;
                }
            }
        }
    }

    if(! msg.empty())
    {
        QStringList list;
        for (std::map<QString,QStringList>::iterator it=msg.begin(); it!=msg.end(); ++it)
            list << it->first + " in " + it->second.join(',');
        int answer = QMessageBox::warning(editor,tr("Warning"),"Unknown files exist:\n" + list.join('\n'),QMessageBox::Discard|QMessageBox::Save,QMessageBox::NoButton);

        if(answer == QMessageBox::Discard)
        {
            QStringList list;
            foreach (const QString &it, pathList) {
                if(! QFile(it).remove())
                    list << it;
            }

            if(! list.isEmpty())
            {
                QMessageBox::critical(editor,tr("Error"),"Failed to clear the unknown files:\n" + list.join('\n') + ".\n\nPlease delete them manually.",QMessageBox::Ok,QMessageBox::Ok);
                QDesktopServices::openUrl(QUrl::fromLocalFile( simdatadir.absolutePath()));
            }
        }
    }
}

QString ParoutModel::keyPath(const QString &key) const
{ return simdatadir.absoluteFilePath(key); }


QString ParoutModel::filePath(const QString &key, const QString &fileName) const
{
    QDir dir(simdatadir.absoluteFilePath(key) );
    if(! dir.exists(fileName) )
        return QString();

    return dir.absoluteFilePath(fileName);
}

QStringList ParoutModel::filePathList(const QString &key,const QString &fileName) const
{
    QDir dir(simdatadir.absoluteFilePath(key));
    int pos = fileName.lastIndexOf('.');
    if(pos == -1) pos = fileName.size();

    QString fileFilter = fileName;
    fileFilter.insert(pos,"_*");

    QStringList result;
    foreach (const QFileInfo &it, dir.entryInfoList(QStringList(fileFilter),QDir::Files)) {
        const QString &baseName = it.completeBaseName();
        if(baseName.mid(pos+1).contains(QRegExp("^\\d+$")) )
            result << it.absoluteFilePath();
    }
    return result;
}

std::vector<double> ParoutModel::readStFile(const QString &key, const QString &stFile) const
{
    QString filepath = this->filePath(key,stFile);
    QByteArray temp = filepath.toLocal8Bit();
    std::ifstream _in(temp.data());
    if(!_in) return std::vector<double>();
    std::istream_iterator<double> end;         // end-of-stream iterator
    std::istream_iterator<double> beg(_in);   // stdin iterator
    std::vector<double> result(beg,end);
    if(!_in.eof()) return std::vector<double>();
    return result;
}

bool ParoutModel::existSimulation(const QString &key) const
{ return simdatadir.exists(key);}

bool ParoutModel::generateMain(const QString &mainsource,const QString &path) const
{
    QFile rf(mainsource);
    if(!rf.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(editor,tr("Error"),tr("Can't read the file:\n") + mainsource,QMessageBox::Ok,QMessageBox::Ok);
        return false;
    }
    QString omain = rf.readAll();

    QDir dir(path);
    QFile fm(dir.absoluteFilePath("main.cpp"));

    if(!fm.open(QIODevice::WriteOnly|QIODevice::Truncate) )
    {
        QMessageBox::critical(editor,tr("Error"),tr("Can't create the file:\n") + fm.fileName(),QMessageBox::Ok,QMessageBox::Ok);
        return false;
    }

    for (int i = 0; i < this->rowCount(); ++i)
    {
        const QString para = this->index(i,0).data().toString();
        QRegExp rx("\\b"+para+"\\s*=\\s*\\?\\s*;");
        omain.replace(rx,para+" = "+this->index(i,0).data(Item::DataRole).toString()+";");
    }

    fm.write(omain.toLocal8Bit());
    return true;
}
