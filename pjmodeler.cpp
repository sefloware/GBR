/** @file pjmodeler.cpp
* @brief the modeling interface of one project.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the modeler Class, which is
* used for modeling.
*/

#include "pjmodeler.h"
#include "mdparout.h"
#include "mdeditor.h"
#include "compenv.h"
#include "config.h"
#include "mdparoutdelegate.h"
#include "mddebug.h"
#include "inputdialog.h"
#include "qrcfile.h"
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QToolButton>
#include <QToolBar>
#include <QToolTip>
#include <QMessageBox>
#include <QListView>
#include <QSplitter>
#include <QAction>
#include <QDate>
#include <QDir>
#include <QLabel>

PjModeler::PjModeler(const QDir &dir, QWidget *parent, QSqlDatabase db) :
    QWidget(parent),
    dir(dir)
{
    if( (  ! dir.exists("main.cpp")) &&
           ! QFile::copy( ":/file/files/main.cpp", dir.absoluteFilePath("main.cpp")) )
    {
        setEnabled(false);
        QMessageBox::critical(this,tr("Error"),"Failed to build the <b>main.cpp</b> file!",QMessageBox::Ok,QMessageBox::Ok);
    }

    //editor
    editor = new MainEditor;
    editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    regionViewer = new QListView;
    regionViewer->setObjectName("RegionViewer");
    regionModel = new QStandardItemModel(this);
    mdDebug = new MdDebug;
    paroutModel = new ParoutModel(dir,editor,db);
    paroutViewer = new ParoutView;

    titleLabel = new QLabel;
    titleLabel->setObjectName("TitleLabel");
    titleLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    titleLabel->setText("Simulation Datas exist! (Read-Only Mode)");

    editButton = new QToolButton;
    editButton->setText(tr("Enable Editing") );
    editButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    editButton->setObjectName("Switcher");
    editButton->setCheckable(true);
    connect(editButton, SIGNAL(toggled(bool)), this,SLOT(setReadWrite(bool)) );

    nextButton = new QToolButton;
    nextButton->setIcon(QIcon(":/icon/images/next.png") );
    nextButton->setText(tr("Next Page"));
    nextButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    nextButton->setToolTip(tr("Next Page"));
    nextButton->setObjectName("ToolButton");

    addchainButton = new QToolButton;
    addchainButton->setText(tr("Chain"));
    addchainButton->setIcon(QIcon(":/icon/images/addchain.png") );
    addchainButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addchainButton->setToolTip(tr("New Chain"));
    addchainButton->setObjectName("ToolButton");

    addoutputButton = new QToolButton;
    addoutputButton->setText(tr("Output"));
    addoutputButton->setIcon(QIcon(":/icon/images/addoutput.png") );
    addoutputButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addoutputButton->setToolTip(tr("New Output"));
    addoutputButton->setObjectName("ToolButton");


    titleBar = new QToolBar;
    titleBar->setObjectName("TitleBar");
    editAction = titleBar->addWidget(editButton);
    titleBar->addWidget(titleLabel);

    deleteAction = titleBar->addAction(QIcon(":/icon/images/delete.png"),tr("Delete"),this,SLOT(deleteExtraSelections()));
    deleteAction->setDisabled(true);
    undoAction = titleBar->addAction(QIcon(":/icon/images/undo.png"),tr("Undo"),editor,SLOT(undo()));
    undoAction->setDisabled(true);
    redoAction = titleBar->addAction(QIcon(":/icon/images/redo.png"),tr("Redo"),editor,SLOT(redo()));
    redoAction->setDisabled(true);
    saveAction = titleBar->addAction(QIcon(":/icon/images/save.png"),tr("Save"),this,SLOT(save()) );
    saveAction->setDisabled(true);
    titleBar->addSeparator();
    titleBar->addWidget(addchainButton);
    titleBar->addWidget(addoutputButton);
    titleBar->addSeparator();
    titleBar->addWidget(nextButton);

    //left splitter
    QSplitter *spl = new QSplitter(Qt::Vertical);
    spl->addWidget(regionViewer);
    spl->addWidget(paroutViewer);

    //rigth splitter
    QSplitter *spr = new QSplitter(Qt::Vertical);
    spr->addWidget(editor);
    spr->addWidget(mdDebug->textOutput);

    QVBoxLayout *lr = new QVBoxLayout;
    lr->addWidget(spr);
    lr->addWidget(mdDebug);

    QHBoxLayout *bodyLayout = new QHBoxLayout;
    bodyLayout->addWidget(spl);
    bodyLayout->addLayout(lr);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titleBar);
    mainLayout->addLayout(bodyLayout);

    QStringList conststrs;
    conststrs << "//!" << "for(unsigned long istep=0;istep<steps;++istep)";
    QFile file( dir.absoluteFilePath("main.cpp"));
    file.setPermissions( QFileDevice::WriteOwner);
    editor->setEnabled( file.open(QIODevice::ReadOnly));
    editor->setPlainText( file.readAll());
    editor->setConstStrings( conststrs);
    editor->updateEnv();
    editor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //region widget
    regionViewer->setModel(regionModel);
    regionViewer->setSelectionMode(QAbstractItemView::ExtendedSelection);
    regionViewer->setEditTriggers(QAbstractItemView::NoEditTriggers);
    readRegion();

    //parout viewer.
    connect(paroutModel,SIGNAL(simulationExistence(bool)),this,SLOT(setReadOnlyMode(bool)) );

    paroutModel->initialize();
    paroutViewer->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked);
    paroutViewer->setSelectionMode(QAbstractItemView::SingleSelection);
    paroutViewer->setContextMenuPolicy(Qt::ActionsContextMenu);
    paroutViewer->setIndentation(0);
    paroutViewer->setAlternatingRowColors(true);
    paroutViewer->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    paroutViewer->header()->setStretchLastSection(false);
    paroutViewer->header()->setMinimumSectionSize(15);
    paroutViewer->setModel(paroutModel);
    paroutDelegate = new ParoutDelegate(paroutViewer);
    paroutViewer->setItemDelegateForColumn(1,paroutDelegate);
    paroutViewer->resizeToContent();

    connect(undoAction,SIGNAL(triggered()),this,SLOT(readRegion()));
    connect(redoAction,SIGNAL(triggered()),this,SLOT(readRegion()));
    connect(addchainButton,SIGNAL(clicked()),this,SLOT(newChain()) );
    connect(addoutputButton,SIGNAL(clicked()),this,SLOT(newOutput()) );

    connect(regionViewer->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(regionViewerSelectionChanged()) );
    connect(nextButton,SIGNAL(clicked()),this,SIGNAL(switch2Analyse()) );
    connect(editor,SIGNAL(undoAvailable(bool)),undoAction,SLOT(setEnabled(bool)) );
    connect(editor,SIGNAL(redoAvailable(bool)),redoAction,SLOT(setEnabled(bool)) );
    connect(editor,SIGNAL(blockCountChanged(int)),this,SLOT(highlightCurrentSelection()) );
    connect(mdDebug,SIGNAL(runTriggered()),this,SLOT(debug()) );
    connect(mdDebug,SIGNAL(exploreDebugFolder()),this,SLOT(exploreDebugFolder()) );
    connect(editor,SIGNAL(modificationChanged(bool)),this,SLOT(slotModificationChanged(bool)) );
}

void PjModeler::slotModificationChanged(bool modified)
{
    mdDebug->setReadOnly(modified);
    saveAction->setEnabled(modified);
    this->setWindowModified(modified);
    emit modificationChanged(modified);
}

void PjModeler::exploreDebugFolder()
{
    QDesktopServices::openUrl( QUrl::fromLocalFile(dir.absoluteFilePath("debug")));
}

void PjModeler::regionViewerSelectionChanged()
{
    QModelIndexList list = regionViewer->selectionModel()->selectedIndexes();
    highlightIndexs(list);

    if(editor->isReadOnly()) return;

    if(list.isEmpty())
    {
        deleteAction->setDisabled(true);
        return;
    }

    bool disable = false;
    foreach (const QModelIndex &it, list)
        if(it.row() < 2)
        {
            disable = true;
            break;
        }
    deleteAction->setDisabled(disable);
}

void PjModeler::deleteExtraSelections()
{
    QList<QTextCursor> list;
    foreach(const QTextEdit::ExtraSelection &it,editor->extraSelections() )
        list << it.cursor;

    qSort(list.begin(),list.end(),qGreater<QTextCursor>());
    QTextCursor tc = list.takeFirst();
    tc.beginEditBlock();
    tc.select(QTextCursor::BlockUnderCursor);
    tc.removeSelectedText();
    tc.endEditBlock();
    foreach (QTextCursor it, list)
    {
        it.select(QTextCursor::BlockUnderCursor);
        it.joinPreviousEditBlock();
        it.removeSelectedText();
        it.endEditBlock();
    }
    readRegion();
    highlightCurrentSelection();
}

void PjModeler::readRegion()
{
    assert(editor != 0);

     QRegExp rx("//! \\[\\w+\\b(?:\\[[^]]*\\])?(?=[^]]*\\])");
     QTextCursor tc(editor->document());

     regionModel->clear();

     QStringList list;
     while (! (tc = editor->document()->find(rx,tc,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords)).isNull() )
         list << tc.selectedText();
     list.removeDuplicates();
     list.removeAll("//! [Parameter");
     list.removeAll("//! [Output");

     QStandardItem *ip = new QStandardItem("Parameter");
     ip->setForeground(Qt::red);
     ip->setData(Qt::AlignCenter,Qt::TextAlignmentRole);
     regionModel->appendRow(ip);

     QStandardItem *io = new QStandardItem("Output");
     io->setForeground(Qt::red);
     io->setData(Qt::AlignCenter,Qt::TextAlignmentRole);
     regionModel->appendRow(io);

     foreach (const QString &it, list) {
         QStandardItem *item = new QStandardItem(it.mid(5));
         item->setForeground(Qt::blue);
         item->setData(Qt::AlignCenter,Qt::TextAlignmentRole);
         regionModel->appendRow(item);
     }

}


QList<QTextEdit::ExtraSelection> PjModeler::transformToExtraSelection(const QList<QTextCursor> &tcs)
{
    QList<QTextEdit::ExtraSelection> result;
    foreach (const QTextCursor &it, tcs) {
        result << transformToExtraSelection(it);
    }
    return result;
}

QTextEdit::ExtraSelection PjModeler::extraSelection(const QTextCursor &tc)
{
    assert(!tc.isNull());
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::white).lighter();
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = tc;
    selection.cursor.clearSelection();
    return selection;
}

QList<QTextEdit::ExtraSelection>  PjModeler::transformToExtraSelection(QTextCursor tc)
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if(tc.isNull())
        return extraSelections;
    assert(tc.anchor() < tc.position());

    QTextCursor it(tc);
    it.setPosition(tc.anchor());
    it.movePosition(QTextCursor::StartOfBlock);

    tc.movePosition(QTextCursor::StartOfBlock);
    extraSelections.append(extraSelection(tc));

    while(it != tc)
    {
        extraSelections.append(extraSelection(it));
        if(!it.movePosition(QTextCursor::NextBlock) ) break;
    }
    return extraSelections;
}

void PjModeler::highlightIndexs(const QModelIndexList &indexs)
{
    QList< QTextEdit::ExtraSelection> list;
    foreach (const QModelIndex &it, indexs) {
        list << transformToExtraSelection(editor->specialRegionsOf(it.data().toString()));
    }

    editor->viewport()->setStyleSheet(list.isEmpty() ? "background-color: white;" : "background-color: #D5D5F9;");
    editor->setExtraSelections(list);
}

void PjModeler::highlightCurrentSelection()
{ highlightIndexs(regionViewer->selectionModel()->selectedIndexes()); }

bool PjModeler::containRegion(QString secName) const
{
    const QRegExp rx("\\[[^\\]]*\\]");
    secName.remove(rx);
    for(int i=0;i<regionModel->rowCount();++i)
    {
        QString old = regionModel->index(i,0).data().toString();
        old.remove(rx);
        if( old == secName)
            return true;
    }
    return false;
}

void PjModeler::setReadOnlyMode(bool ro)
{
    editAction->setVisible(ro);
    editButton->setChecked(!ro);

    setReadWrite(! ro);
}

void PjModeler::setReadWrite(bool rw)
{
    if(! rw && (isWindowModified() && ! save()) )
    {
        QToolTip::showText(QCursor::pos(),"Failed to disable editing!");
        editButton->setChecked(true);
        return;
    }

    editButton->setText(rw ? "Disable Editing" : "Enable Editing");

    if(! editAction->isVisible())
        titleLabel->setText("Simulation Datas don't exist! (Read-Write Mode)");
    else
        titleLabel->setText(rw ? "Simulation Datas exist! (Read-Write Mode)" : "Simulation Datas exist! (Read-Only Mode)");

    editor->setReadOnly(! rw);
    if(! rw) editor->document()->clearUndoRedoStacks();

    if(! rw)
    {
        deleteAction->setEnabled(false);
        undoAction->setEnabled(false);
        redoAction->setEnabled(false);
    }
    addchainButton->setEnabled(rw);
    addoutputButton->setEnabled(rw);
}

void PjModeler::debug()
{
    if( ! insureDirectory(dir.absoluteFilePath("debug") ) )
        return;

    if(!paroutModel->generateMain(dir.absoluteFilePath("main.cpp"),dir.absoluteFilePath("debug")) )
        return;

    QString cc = CE::debugCompilerCommand(mdDebug->currentIndex() );
    if(cc.isEmpty())
        return;

    mdDebug->setWorkingDirectory(dir.absoluteFilePath("debug"));
    mdDebug->run(QStringList() << cc << mdDebug->workingDirectory().absoluteFilePath("out.exe"));
}

bool PjModeler::newChain()
{
    QRegExp regExp("[a-zA-z_]\\w*(?:\\[\\d+\\])?");
    QRegExpValidator v(regExp, 0);

    QString name= InputDialog::getText(this, tr("New Chain"), tr("Name: "), QString(), &v );
    if(name.isEmpty())
        return false;
    if(containRegion(name))
    {
        QMessageBox::critical(this,tr("Error"),"<b>" + name + "</b> has been used!",QMessageBox::Ok,QMessageBox::Ok);
        return false;
    }

    QRCFile file(":/file/files/chain.cpp");
    file.open();

    QRegExp rx("^([a-zA-z_]\\w*)\\[(\\d+)\\]$");
    QString region0,region1;
    if(name.contains(rx))
    {
        region0 = file.context("array",0).arg(name, rx.cap(1), rx.cap(2));
        region1 = file.context("array",1).arg(name, rx.cap(1), rx.cap(2));
    }
    else
    {
        region0 = file.context("one",0).arg(name);
        region1 = file.context("one",1).arg(name);
    }
    bool ok;

    QTextCursor tc = editor->document()->find("//! [Output]",0,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
    assert(!tc.isNull());
    ok = tc.movePosition(QTextCursor::PreviousBlock);
    assert(ok);
    tc.movePosition(QTextCursor::EndOfBlock);

    tc.beginEditBlock();
    tc.insertText("\n");
    editor->formattedInsertText(region0,tc);

    QTextCursor tc1 = editor->document()->find("//! [Output lincs]",0,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
    assert(!tc1.isNull());
    ok = tc1.movePosition(QTextCursor::PreviousBlock);
    assert(ok);
    tc1.movePosition(QTextCursor::EndOfBlock);

    tc.setPosition(tc1.position());
    tc.insertText("\n");
    editor->formattedInsertText(region1,tc);

    tc.endEditBlock();

    QStandardItem *item = new QStandardItem(name);
    item->setForeground(Qt::blue);
    item->setData(Qt::AlignCenter,Qt::TextAlignmentRole);
    regionModel->appendRow(item);
    regionViewer->setCurrentIndex(item->index());
    return true;
}


bool PjModeler::newOutput()
{
    OutputNameInputDialog dialog(this);
    if(! dialog.exec())
        return true;

    QString name= dialog.getText();
    assert(! name.isEmpty());

    int pos = name.indexOf('.');
    assert(pos != -1);
    QString suffix = name.mid(pos+1);

    bool ok;

    QTextCursor tc = editor->document()->find("//! [Output end]",0,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
    ok = tc.movePosition(QTextCursor::PreviousBlock);
    assert(ok);
    tc.movePosition(QTextCursor::EndOfBlock);

    tc.beginEditBlock();
    tc.insertText("\n");

    QString def,str;
    if(suffix == "st")
    {
        def = "std::ofstream f%1(\"%2\",std::ios::app);\nlong double %1 = 0.0;";
        str = "f%1 << ... << std::endl;";
    }
    else if(suffix == "bin")
    {
        def = "std::ofstream f%1(\"%2\",std::ios::binary);";
        str = "f%1.write((char*)(/*chain*/.data()),sizeof(double)*3*/*beadCount*/);";
    }
    else
    {
        def = "std::ofstream f%1(\"%2\");\nlong double %1 = 0.0;";
        str = "f%1 << ... << std::endl;";
    }

    editor->formattedInsertText(def.arg(name.left(pos), name),tc);

    QTextCursor tc2;
    if(suffix==QLatin1String("st"))
    {
        tc2 = editor->document()->find("//! [Output out end]",0,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
        ok = tc2.movePosition(QTextCursor::PreviousBlock);
        assert(ok);
        tc2.movePosition(QTextCursor::EndOfBlock);
    }
    else
    {
        tc2 = editor->document()->find("//! [Output lincs end]",0,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
        if(tc2.isNull())
        {
            QToolTip::showText(QCursor::pos(),"Failed to locate the <b>//! [Output lincs end]</b> structure.");
            return false;
        }
        ok = tc2.movePosition(QTextCursor::PreviousBlock,QTextCursor::MoveAnchor,2);
        assert(ok);
        tc2.movePosition(QTextCursor::EndOfBlock);
    }

    tc.setPosition(tc2.position());
    tc.insertText("\n");
    editor->formattedInsertText( str.arg(name.left(pos)), tc);
    tc.endEditBlock();

    assert(regionModel->rowCount() > 1);
    regionViewer->setCurrentIndex(regionModel->index(1,0));
    return true;
}

void PjModeler::saveTemporary()
{
    paroutModel->saveHeaders();
    paroutModel->saveParout();
}

bool PjModeler::save()
{
    if (! isWindowModified())
        return true;

    QFile fm(dir.absoluteFilePath("main.cpp"));
    QFile fv(dir.absoluteFilePath("time.stamp"));

    if(! fm.open(QIODevice::WriteOnly|QIODevice::Truncate) || ! fv.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        QMessageBox::warning(this,tr("Error"),tr("Failed to open the <b>time.stamp</b> or <b>main.cpp</b>!"), QMessageBox::Ok, QMessageBox::Ok);
        return false;
    }

    fm.write(editor->toPlainText().toLocal8Bit());

    QString currentData = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    fv.write( currentData.toLatin1());

    editor->document()->setModified(false);
    editor->updateObjModel();
    paroutModel->reselect();
    paroutViewer->resizeToContent();
    return true;
}

