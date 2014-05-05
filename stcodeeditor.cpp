/** @file stcodeeditor.cpp
* @brief the source code editor in the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the source code editor which
* is composed of the edit bar, the Editor2 object,
* the C++ objects model and view.
*/

#include "stcodeeditor.h"
#include "assert.h"
#include "stcode.h"
#include "editor2.h"
#include "cppobjlist.h"
#include "cppreader.h"
#include "cppclass.h"
#include "cppvariable.h"
#include "cppenumespace.h"
#include "cppfunction.h"
#include "stscparser.h"
#include <QTreeView>
#include <QFileSystemModel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QIcon>
#include <QAction>
#include <QToolBar>
#include <QToolButton>
#include <QStandardItemModel>
#include <QLabel>
#include <QMessageBox>
#include <QToolTip>

CodeEditor::CodeEditor(const QString &codePath, Codist *codist, QWidget *parent) :
    QWidget(parent),refCodist(codist)
{
    QWidget::setWindowFilePath(codePath);

    labelTitle = new QLabel;
    labelTitle->setObjectName("TitleLabel");
    labelTitle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    switcher = new QToolButton;
    switcher->setText(tr("Enable Editing"));
    switcher->setToolButtonStyle(Qt::ToolButtonTextOnly);
    switcher->setObjectName("Switcher");
    switcher->setCheckable(true);

    model = new QStandardItemModel;

    view = new QTreeView;
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    view->setHeaderHidden(true);
    view->setObjectName("PlainList");
    view->setWordWrap(true);

    titleBar = new QToolBar;
    titleBar->setObjectName("TitleBar");

    edit = new Editor2;

    titleBar->addWidget(labelTitle);

    redoAction = titleBar->addAction(QIcon(":/icon/images/redo.png"),tr("&Redo"),edit,SLOT(redo()) );
    redoAction->setEnabled(false);
    undoAction = titleBar->addAction(QIcon(":/icon/images/undo.png"),tr("&Undo"),edit,SLOT(undo()) );
    undoAction->setEnabled(false);
    saveAction = titleBar->addAction(QIcon(":/icon/images/save.png"),tr("&Save"),this,SLOT(save()) );
    saveAction->setEnabled(false);
    generateAction = titleBar->addAction(QIcon(":/icon/images/generatecode.png"),tr("&Generate Code"),this,SLOT(generateRoadblockCode()) );
    generateAction->setEnabled(false);

    switchAction = titleBar->addWidget(switcher);

    QSplitter *sh = new QSplitter(Qt::Horizontal);
    sh->addWidget(view);
    sh->addWidget(edit);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(titleBar);
    layout->addWidget(sh);

    //! TreeView
    connect(edit,SIGNAL(modificationChanged(bool)),this,SLOT(setWindowModified(bool)) );
    connect(edit,SIGNAL(redoAvailable(bool)),redoAction,SLOT(setEnabled(bool)) );
    connect(edit,SIGNAL(undoAvailable(bool)),undoAction,SLOT(setEnabled(bool)) );
    connect(edit,SIGNAL(modificationChanged(bool)),saveAction,SLOT(setEnabled(bool)) );
    connect(edit,SIGNAL(modificationChanged(bool)),generateAction,SLOT(setEnabled(bool)) );
    connect(edit,SIGNAL(modificationChanged(bool)),this,SLOT(setModification(bool)) );

    connect(switcher,SIGNAL(toggled(bool)),this,SLOT(setReadWrite(bool)) );

    if(! load(codePath))
    {
        this->setEnabled(false);
        return;
    }

    setModification(false);
    setReadWrite(false);
}

CodeEditor::~CodeEditor()
{ model->deleteLater(); }

void CodeEditor::setWindowFilePath(const QString &path)
{
    QWidget::setWindowFilePath(path);

    QString title = this->windowTitle();
    title.replace("[*]",isWindowModified() ? "*" : "");
    labelTitle->setText(title);
}

void CodeEditor::updateEditorEnvironment()
{ edit->updateEnvModel(); }

void CodeEditor::select(QStandardItem *parent,const QStandardItem *sourceParent)
{
    for (int i=0; i<sourceParent->rowCount(); ++i)
    {
       QStandardItem *child = sourceParent->child(i);

       QStandardItem *item = child->QStandardItem::clone();
       item->setToolTip(child->toolTip());
       assert(item);

       QString text = child->data(Item::DescriptionRole).toStringList().join('\n');
       QStandardItem *desitem = new QStandardItem(text);
       desitem->setForeground(Qt::darkGreen);
       desitem->setEditable(false);

       parent->appendRow(QList<QStandardItem *>() << item << desitem);

       if(child->hasChildren())
           select(item,child);
    }
}

bool CodeEditor::load(const QString &path)
{
    QFileInfo info(path);
    this->setEnabled(info.isReadable());

    bool exc = QFile::exists( info.absoluteFilePath().append('c'));
    switchAction->setVisible(! exc && info.isWritable());

    if(Editor1::source().contains(info.fileName()))
    {
        select(model->invisibleRootItem(),Editor1::source().value(info.fileName())->invisibleRootItem());
        view->setModel(model);
        view->expandAll();
        view->resizeColumnToContents(0);
    }

    QFile file( path);
    if(! file.open(QIODevice::ReadOnly))
    {
        this->setEnabled(false);
        return false;
    }

    QString text = file.readAll();
    edit->setPlainText(text);
    edit->updateEnv();
    edit->setFocusPolicy(Qt::StrongFocus);
    edit->setHidden(true);

    redoAction->setVisible(false);
    undoAction->setVisible(false);
    saveAction->setVisible(false);
    generateAction->setVisible(false);
    return true;
}

void CodeEditor::setReadWrite(bool rw)
{
    if(! rw && (isWindowModified() && ! this->maybeSave()) )
    {
        QToolTip::showText(QCursor::pos(),"Failed to disable editing!");
        switcher->setChecked(true);
        return;
    }

    switcher->setText(rw ? "Disable Editing" : "Enable Editing");

    redoAction->setVisible(rw);
    undoAction->setVisible(rw);
    saveAction->setVisible(rw);
    generateAction->setVisible(rw);

    edit->setVisible(rw);
}

void CodeEditor::setModification(bool modified)
{
    setWindowModified(modified);

    QString title = this->windowTitle();
    title.replace("[*]",modified ? "*" : "");
    labelTitle->setText(title);

    refCodist->setModification(this->windowFilePath(),modified);
}

bool CodeEditor::save()
{
    QFile file(windowFilePath());
    if(!file.open(QIODevice::WriteOnly| QIODevice::Truncate) )
        return false;

    const QString text = edit->toPlainText();
    file.write(text.toLocal8Bit());
    file.close();

    CppObjList objList;
    CppReader reader(text);
    objList << reader;

    objList.readDescription(text);
    if(!objList.isEmpty())
    {
        QFileInfo info(windowFilePath());

        if( Editor1::source().contains(info.fileName()))
        {
            QStandardItemModel *objModel = Editor1::source().value(info.fileName());
            objModel->clear();
            foreach (QStandardItem *it, objList)
                objModel->appendRow( it);
            objModel->setHorizontalHeaderLabels(QStringList(info.fileName()));

            model->clear();
            select(model->invisibleRootItem(),objModel->invisibleRootItem());
            view->expandAll();
            view->resizeColumnToContents(0);
        }
        else
        {
            QStandardItemModel *objModel = new QStandardItemModel;
            foreach (QStandardItem *it, objList)
                objModel->appendRow( it);
            objModel->setHorizontalHeaderLabels(QStringList(info.fileName()));
            Editor1::source().insert(info.fileName(),objModel);

            model->clear();

            select(model->invisibleRootItem(),objModel->invisibleRootItem());
            view->expandAll();
            view->resizeColumnToContents(0);
        }
    }

    edit->document()->setModified(false);
    return true;
}

bool CodeEditor::parsePlane(const QString &record, client::dimenmap &v)
{
    if(record.isEmpty())
        return false;

    QByteArray data = record.toLocal8Bit();
    std::string line(data.data());


    using boost::spirit::ascii::space;
    std::string::iterator iter = line.begin();
    std::string::iterator end = line.end();
    client::surface result;
    client::PlaneGrammar<std::string::iterator> pg;
    if(! phrase_parse(iter, end, pg, space, result) )
        return false;

    v.insert(result.todimenssion());
    return true;
}

bool CodeEditor::parseAssemble(const QString &assemble, client::expressiontype &exp)
{
    if(assemble.isEmpty())
        return false;

    QByteArray data = assemble.toLocal8Bit();
    std::string line(data.data());

    using boost::spirit::ascii::space;
    std::string::iterator iter = line.begin();
    std::string::iterator end = line.end();
    client::LogcalGrammar<std::string::iterator> lg;
    return phrase_parse(iter, end, lg, space, exp);
}

bool CodeEditor::generateRoadblockCode()
{
    assert( edit);
    QTextCursor tcbeg(edit->document());
    while(!(tcbeg=edit->find("//! Roadblock Function Template", tcbeg, QTextDocument::FindCaseSensitively)).isNull())
    {
        tcbeg.movePosition(QTextCursor::NextBlock,QTextCursor::MoveAnchor,2);
        if(edit->document()->characterAt(tcbeg.position()) != QLatin1Char('{'))
            continue;

        QTextCursor tcend = edit->findPair('}',tcbeg);
        if(tcend.isNull())
            return false;

        QTextCursor tc = tcbeg;

        tcbeg = edit->find(QRegExp("//!\\s*#\\s*Assemble\\s*="),tcbeg,QTextDocument::FindCaseSensitively);
        if( tcbeg.isNull())
        {
            tcbeg = tc;
            continue;
        }
        tcbeg.clearSelection();
        tcbeg.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);

        client::expressiontype assemble;
        QString tmp = tcbeg.selectedText().simplified();
        tmp.replace("-","&&!");
        if(! parseAssemble(tmp,assemble) )
        {
            edit->setTextCursor(tcbeg);
            QPoint point = edit->viewport()->mapToGlobal(edit->cursorRect(tcbeg).bottomRight());
            QToolTip::showText(point,"In the line <b>" + QString::number(tcbeg.blockNumber()+1) + "</b> exists wrong inputting!");
            return false;
        };


        tcbeg.movePosition(QTextCursor::NextBlock);

        client::dimenmap  dimap;
        while( !(tc = edit->find(QRegExp("//!\\s*@\\s*\\w+\\s*\\{"),tc,QTextDocument::FindCaseSensitively)).isNull() && tc.position() < tcend.position() )
        {
            tc.select(QTextCursor::BlockUnderCursor);
            if(! parsePlane(tc.selectedText().simplified(),dimap) )
            {
                edit->setTextCursor(tc);
                QPoint point = edit->viewport()->mapToGlobal(edit->cursorRect(tc).bottomRight());
                QToolTip::showText(point,"In the line <b>" + QString::number(tc.blockNumber()+1) + "</b> exists wrong inputting!");
                return false;
            }
        }

        tc = edit->find("}",tcbeg);
        if(tc.isNull() || tc.position() >= tcend.position())
            return false;
        tcbeg.setPosition(tc.anchor(),QTextCursor::KeepAnchor);

        std::string conditiontext = client::expressionconditiontext(assemble,dimap);

        tcbeg.beginEditBlock();
        tcbeg.insertBlock();
        edit->formattedInsertText(QString::fromStdString(conditiontext),tcbeg);
        tcbeg.insertText(edit->indentationOf(tcbeg,false).prepend('\n'));
        tcbeg.endEditBlock();
    }
    return true;
}

bool CodeEditor::maybeSave()
{
    if (!edit->document()->isModified())
        return true;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Unsaved Changes Warning"), tr("The document has been changed."),
                               QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::No:
        return load(windowFilePath());
    case QMessageBox::Cancel:
        edit->setFocus();
        return false;
    default:
        return true;
    }
    return true;
}
