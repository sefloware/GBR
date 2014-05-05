/** @file mdeditor.cpp
* @brief the modeling editor.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the modeling editor Class.
*/

#include "mdeditor.h"
#include "assert.h"
#include "editorobjmodel.h"
#include "cppobjlist.h"
#include "cppreader.h"
#include <QSortFilterProxyModel>
#include <QMimeData>
#include <QTextBlock>
#include <QToolTip>
#include <QToolBar>
#include <QAction>
#include <QStandardItemModel>
#include <QTextDocumentFragment>

MainEditor::MainEditor(QWidget *parent) :
    Editor2(parent), readonly(false)
{
    objModel = new QStandardItemModel;
    objectModel->appendPersistentModel(objModel);

    connect(this,SIGNAL(cursorPositionChanged()),this,SLOT(updateRWstateUponCursor()) );
    connect(this,SIGNAL(selectionChanged()),this,SLOT(updateRWstateUponSelection()) );
}

void MainEditor::updateEnv()
{
    updateObjModel();
    Editor2::updateEnv();
}

QTextCursor MainEditor::selectRegion(const QRegExp &begin,const QRegExp &end, const QTextCursor &from)
{
    assert(begin.isValid() && end.isValid());
    QTextCursor tcfir = document()->find(begin,from,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
    QTextCursor tcsec = document()->find(end,tcfir,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
    if(tcfir.isNull() || tcsec.isNull()) return QTextCursor();
    tcfir.setPosition(tcsec.position(),QTextCursor::KeepAnchor);
    return tcfir;
}

QTextCursor MainEditor::selectRegion(const QString &begin,const QString &end, const QTextCursor &from)
{
    assert(!begin.isEmpty() && !end.isEmpty());
    QTextCursor tcfir = document()->find(begin,from,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
    QTextCursor tcsec = document()->find(end,tcfir,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
    if(tcfir.isNull() || tcsec.isNull()) return QTextCursor();
    tcfir.setPosition(tcsec.position(),QTextCursor::KeepAnchor);
    return tcfir;
}

QList<QTextCursor> MainEditor::specialRegionsOf(const QString &name)
{
    QRegExp rxbeg("//! \\["+QRegExp::escape(name)+"(?: .*)?\\]");
    QRegExp rxend("//! \\["+QRegExp::escape(name)+" .*\\bend\\]");
    QList<QTextCursor> result;
    QTextCursor tc;
    while( !(tc=selectRegion(rxbeg,rxend,tc)).isNull())
        result << tc;
    return result;
}

QTextCursor MainEditor::selectStruct(const QString &header, const QTextCursor &from, bool includeBorder)
{
    QTextCursor tc = find(header,from,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
    if(tc.isNull()) return tc;

    QTextCursor tmp(tc);
    while(document()->characterAt(tmp.position()).isSpace() ||
          document()->characterAt(tmp.position()) == QChar::Separator_Paragraph)
        tmp.movePosition(QTextCursor::NextCharacter);
    if(document()->characterAt(tmp.position())!=QLatin1Char('{'))
        return QTextCursor();

    if(!includeBorder)
        tc.setPosition(tmp.position()+1);
    tmp = findPair(QLatin1Char('}'),tmp,QTextCursor::NextCharacter);
    if(tmp.isNull())
        return QTextCursor();

    if(includeBorder)
        if(!tmp.movePosition(QTextCursor::NextCharacter))
            return QTextCursor();
    tc.setPosition(tmp.position(),QTextCursor::KeepAnchor);
    return tc;
}

bool MainEditor::canInputCh(const QString &chkey,QString &matchedStr)
{
    const QStringList vs = conststrs.values(chkey);
    assert(!vs.isEmpty());

    foreach (const QString &it, vs) {
        QTextCursor tc = textCursor();
        if(!tc.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,it.length()-1))
            continue;
        if(tc.selectedText() == it.leftRef(it.length()-1))
        {
            matchedStr = it;
            return false;
        }
    }
    return true;
}

void MainEditor::keyPressEvent(QKeyEvent *e)
{
    if ( (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) && isReadOnly())
    {
        e->ignore();
        return;
    }

    Editor2::keyPressEvent(e);

    QString chkey = e->text().right(1);
    QString matchStr;
    if( (conststrs.contains(chkey)) && !canInputCh(chkey,matchStr))
    {
        e->ignore();
        QPoint point = viewport()->mapToGlobal(cursorRect(textCursor()).bottomRight());
        QToolTip::showText(point,"Cann't input <b>"+matchStr+"</b> by handcoding");
        return;
    }
}

bool MainEditor::canInsertFromMimeData(const QMimeData * /*source*/) const
{
    return false;
}

void MainEditor::insertFromMimeData(const QMimeData *source)
{
    foreach (const QString &it, conststrs) {
        if(source->text().contains(it))
            return;
    }
    Editor2::insertFromMimeData(source);
}

void MainEditor::updateRWstateUponSelection()
{
    QTextCursor tc = textCursor();

    int end,begin;
    if(tc.position() < tc.anchor())
    {
        end = tc.anchor();
        begin = tc.position();
    }
    else
    {
        end = tc.position();
        begin = tc.anchor();
    }

    tc.setPosition(begin);
    tc.movePosition(QTextCursor::StartOfBlock);
    tc.setPosition(end,QTextCursor::KeepAnchor);
    tc.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);

    QString str=tc.selectedText();
    setReadOnly(str);

}

void MainEditor::setReadOnly(bool ro)
{
    readonly = ro;
    Editor2::setReadOnly(ro);
}

void MainEditor::setReadOnly(const QString &str)
{
    foreach (const QString &it, conststrs)
        if(str.contains(it))
        {
            Editor2::setReadOnly(true);
            return;
        }
    Editor2::setReadOnly(readonly);
}

void MainEditor::updateRWstateUponCursor()
{
    const QString str = textCursor().block().text();
    setReadOnly(str);
}

void MainEditor::updateObjModel()
{
    objModel->clear();
    CppObjList items;

    QTextCursor tc;
    if(!tc.isNull())
    {
        tc= document()->find("int main()",0,QTextDocument::FindCaseSensitively|QTextDocument::FindWholeWords);
        tc.movePosition(QTextCursor::StartOfBlock);
        tc.setPosition(0,QTextCursor::KeepAnchor);
        QString _text = tc.selection().toPlainText();

        CppReader reader(_text);
        items << reader;
    }

    tc = selectStruct("int main()",QTextCursor(document()),false);
    if(!tc.isNull())
    {
        QString text = tc.selection().toPlainText();
        CppReader reader(text);
        items << reader;
    }

    foreach (QStandardItem *it,items)
         objModel->appendRow(it);
}
