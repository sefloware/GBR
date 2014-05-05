/** @file editor1.cpp
* @brief the C++ editor1.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the the C++ editor Class inherited
* from the QPlainTextEdit Class.
*/

#include "editor1.h"
#include "cppobjlist.h"
#include "cppfunction.h"
#include "cppposquery.h"
#include "cppclass.h"
#include "cppvariable.h"
#include "cppenumespace.h"
#include "editorobjmodel.h"
#include "editorcompleter.h"
#include "cppkeywords.h"
#include "editorhighlighter.h"
#include "config.h"
#include "assert.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QToolTip>
#include <QColor>
#include <QAbstractItemView>
#include <QTextDocumentFragment>
#include <QSortFilterProxyModel>
#include <QShortcut>

CppObjectModelMap &Editor1::source()
{
    static CppObjectModelMap map;
    return map;
}

Editor1::Editor1(QWidget *parent)
    : QPlainTextEdit(parent),
      indentspace("    ")
{
    objectModel = new CppObjectModel(this);
    objectModel->appendPersistentModel(&cppkeys());

    tmc = new CodeCompleter(this);
    tmc->setWidget(this);
    tmc->setWrapAround(false);
    tmc->setCompletionMode(QCompleter::PopupCompletion);
    tmc->setCaseSensitivity(Qt::CaseSensitive);
    tmc->setModelSorting(QCompleter::UnsortedModel);
    tmc->setSeparator(QLatin1String("::"));
    tmc->setModel(objectModel);

    connect(tmc, SIGNAL(activated(QModelIndex)),
            this, SLOT(insertCompletion(QModelIndex)) );
    connect(this,SIGNAL(blockCountChanged(int)),
            this,SLOT(updateEnvModel()) );

    highlighter = new Highlighter(this);

    QShortcut  *m_Ctrl_F_Accel= new QShortcut(QKeySequence(tr("Ctrl+F")), this);
    connect(m_Ctrl_F_Accel,SIGNAL(activated ()),this,SLOT(mark()));
}

void Editor1::mark()
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    QString searchString = tc.selectedText();

    highlighter->setFindString(searchString);
}

void Editor1::insertCompletion(const QModelIndex &index)
{
    if (tmc->widget() != this)
        return;
    QTextCursor tc = textCursor();
    QString prefixsection = tmc->completionPrefix();
    if(!prefixsection.isEmpty())
        prefixsection = prefixsection.section("::",-1,-1);
    QString completionsection = index.data().toString();
    int extra = completionsection.length() - prefixsection.length();
    tc.insertText(completionsection.right(extra));

    if(index.data(Item::CppTypeRole).toInt() == Item::Function)
    {
        tc.insertText("()");
        this->moveCursor(QTextCursor::PreviousCharacter);
        QPoint point = viewport()->mapToGlobal(cursorRect(textCursor()).bottomRight());
        QToolTip::showText(point,index.data(Qt::ToolTipRole).toString());
    }
}

void Editor1::updateEnvModel()
{
    QRegExp rx("#include\\s+[<\"]([^<\"]+)[<\"]");
    QTextCursor tc(document());
    QStringList files;
    while(!(tc=find(rx,tc)).isNull())
    {
        QString text = tc.selectedText().simplified();
        text.remove(0,10).chop(1);
        files << text.trimmed();
    }

    objectModel->clear();
    foreach (const QString &it, files) {
        if(Editor1::source().contains(it))
             objectModel->prependModel(Editor1::source().value(it));
    }
}

QTextCursor Editor1::find(const QString &subString, const QTextCursor &from, QTextDocument::FindFlags options) const
{
    QTextCursor tc = document()->find(subString, from,options);
    if((!tc.isNull()) && tc.charFormat() == highlighter->NCCFormat )
        tc=find(subString,tc,options);
    return tc;
}

QTextCursor Editor1::find(const QString &subString, int from, QTextDocument::FindFlags options) const
{
    QTextCursor tc = document()->find(subString, from,options);
    if((!tc.isNull()) && tc.charFormat() == highlighter->NCCFormat )
        tc=find(subString,tc,options);
    return tc;
}

QTextCursor Editor1::find(const QRegExp &expr, int from, QTextDocument::FindFlags options) const
{
    QTextCursor tc = document()->find(expr, from,options);
    if((!tc.isNull()) && tc.charFormat() == highlighter->NCCFormat )
        find(expr,tc,options);
    return tc;
}

QTextCursor Editor1::find(const QRegExp &expr, const QTextCursor &from, QTextDocument::FindFlags options) const
{
    QTextCursor tc = document()->find(expr, from,options);
    if((! tc.isNull()) && tc.charFormat() == highlighter->NCCFormat )
        find(expr,tc,options);
    return tc;
}

QString Editor1::rtakeOne(QTextCursor &tc) const
{
    //return (), [],  ->, ., word, ::
    tc.clearSelection();
    QRegExp rx("([.)\\]]|->|::|\\b[a-zA-Z_]\\w*)\\s*$");

    if( ! tc.movePosition(QTextCursor::PreviousWord,QTextCursor::KeepAnchor) ||
            ! (tc.selectedText()).contains(rx) )
                return QString();

    tc.setPosition(tc.anchor()-rx.matchedLength());

    QString rxcap1 = rx.cap(1);
    QTextCursor tmp;
    if(rxcap1 == QLatin1String(")"))
        tmp = findPair('(',tc,QTextCursor::PreviousCharacter);
    else if(rxcap1 ==  QLatin1String("]"))
        tmp = findPair('[',tc,QTextCursor::PreviousCharacter);
    else
        return rxcap1;
    tc.setPosition(tc.position()+1);
    if(tmp.isNull())
        return QString();
    tc.setPosition(tmp.position(),QTextCursor::KeepAnchor);

    QString result = tc.selectedText();
    tc.clearSelection();
    return result;
}

Stream Editor1::rtake(QTextCursor tc) const
{
    Stream st;
    QString wd;
    while( !(wd=rtakeOne(tc)).isEmpty() )
    {
        Stream::ItemFlags flag = Stream::flag(wd);

        if( !st.isEmpty() && (
                    ((flag & (Stream::Reference|Stream::Dereference|Stream::ScopeOperator)) && st.flag(0) != Stream::Word ) ||
                    ( flag == Stream::Word && st.flag(0) == Stream::Word ) ||
                    ( (flag & (Stream::RoundOperator|Stream::SquareOperator)) && (st.flag(0) == Stream::Word) )
                    ) )
            return st;
        if( flag == Stream::ScopeOperator)
        {
            QString str;
            do
            {
                if( ! (wd=rtakeOne(tc)). contains(QRegExp("^[a-zA-Z_]\\w*$")) )
                    return Stream();
                str.prepend("::").prepend(wd);
            } while( rtakeOne(tc) == QLatin1String("::") );
            st.prepend(str,Stream::ScopeOperator);
            break;
        }

        st.prepend(wd,flag);
    }

    return st;
}

QString Editor1::takeOne(int &pos,const CppPosQuery &posquery) const
{
    //return (), [],  ->, ., word, ::

    if( pos >= posquery.text().length())
        return QString();

    QRegExp rx("([.[(]|->|::|\\b[a-zA-Z_]\\w*)\\s*");
    if( (posquery.find(rx,pos) ) != pos)
        return QString();

    int tmp;
    QString rxcap1 = rx.cap(1);
    if(rxcap1 == QLatin1String(")"))
        tmp = posquery.findPair(QRegExp("[()]"),pos,CppPosQuery::Quot);
    else if(rxcap1 ==  QLatin1String("]"))
        tmp = posquery.findPair(QRegExp("[[]]"),pos,CppPosQuery::Quot);
    else
    {
        pos += rx.matchedLength();
        return rxcap1;
    }

    if(tmp == -1)
        return QString();

    int anchor = pos;
    pos = tmp + 1;
    return posquery.text().mid(anchor,pos-anchor);
}


Stream Editor1::take(QString str) const
{
    str = CppPosQuery::remove(str,CppPosQuery::Comment).trimmed();
    CppPosQuery posQuery(str);
    int pos=0;
    Stream st;
    QString wd;

    //word::...word::
    QString op, scope;
    int tmp=pos;
    while ( (wd = takeOne(tmp,posQuery)) .contains(QRegExp("^[a-zA-Z_]\\w*$")) &&
            (op = takeOne(tmp,posQuery)) == QLatin1String("::"))
    {
        scope.append(wd).append(op);
        pos = tmp;
    }

    if (! scope.isEmpty())
        st.append(scope,Stream::ScopeOperator);

    while( !(wd = takeOne(pos,posQuery)).isEmpty() )
    {
        Stream::ItemFlags flag = Stream::flag(wd);

        if( (st.isEmpty() && flag != Stream::Word ) || (!st.isEmpty() && (
                    flag == Stream::ScopeOperator ||
                    ((flag & (Stream::Reference|Stream::Dereference)) && (st.rflag(-1) & (Stream::Reference|Stream::Dereference|Stream::ScopeOperator)) ) ||
                    ( flag == Stream::Word && (st.rflag(-1) & (Stream::Word|Stream::RoundOperator|Stream::SquareOperator)) ) ||
                    ( (flag & (Stream::RoundOperator|Stream::SquareOperator)) && (st.rflag(-1) != Stream::Word) )
                    ) ) )
            return Stream();

        st.append(wd,flag);
    }

    return st;
}

QString Editor1::typeOfLocar(const QString &var) const
{
    QRegExp rx("[a-zA-Z0-9_*& ]+\\b"+ var + "\\s*([=(]|(?:\\[\\s*\\w*\\s*\\]\\s*)+)?");

    //locate the definition of var.
    VisibleRegion vr(textCursor(),this);
    QTextCursor tc(document());
    while( ! vr( tc = find(rx,tc,QTextDocument::FindWholeWords | QTextDocument::FindCaseSensitively) ) );
    if(tc.isNull())
        return QString();

   QString def = tc.selection().toPlainText();
   if(def.endsWith('(') || def.endsWith('='))
       def.chop(1);

   VarAttri attri(def);
   if(!attri)
       return QString();
   return attri.typeName();
}

QString Editor1::typeOfNextSection(Stream &stream, const QString &scope) const
{
    //return Variable, Enum;
    if( ! stream.next(Stream::Word))
        return QString();

    QStandardItem * item = objectModel->itemOf(stream.value(),scope);

    QString type;
    if(item)
        switch (item->data(Item::CppTypeRole).toInt())
        {
        case Item::Function:
            if( !stream.next(Stream::RoundOperator))
                return QString();
            type = static_cast<FunctionItem *>(item)->typeOf(argtypes(stream.value()));
            break;
        case Item::Class:
            if ( ! stream.next(Stream::SquareOperator | Stream::RoundOperator))
                return QString();
            type = static_cast<ClassItem *>(item)->typeOf(argtypes(stream.value()) );
            break;
        case Item::Variable:
        case Item::SquareOperator:
        case Item::RoundOperator:
        case Item::TypeKeyword:
        case Item::Enumerator:
            type = item->data(Item::PurifiedTypeRole).toString();
            break;
        default:
            return QString();
        }
    else
        type = typeOfLocar(stream.value());

    while ( !type.isEmpty() && stream.next(Stream::SquareOperator | Stream::RoundOperator) )
        if(type.endsWith('*'))
        {
            if(stream.flag() != Stream::SquareOperator)
                return QString();
            type.chop(1);
        }
        else
        {
            QStandardItem *classitem = objectModel->itemOf(type);
            if( ! classitem || classitem->data(Item::CppTypeRole).toInt() != Item::Class)
                return QString();
            type =  static_cast<ClassItem *>(classitem)->typeOf(argtypes(stream.value()) );
        }

    return type;
}

QString Editor1::argtypes(QString arg) const
{
    assert(!arg.isEmpty());
    QString result = arg.left(1); result.append(arg.right(1));
    arg.remove(0,1).chop(1);

    QStringList arglist = VarAttri::splitArguments(arg);
    QStringList typelist;
    foreach (const QString &it, arglist) {
        if(it.contains(QRegExp("^\\s*[0-9.+-]+\\s*$")))
            typelist << (it.contains('.') ? "double" : "int" );
        else
        {
            Stream st(take(it));
            QString type = scopeOf( st);
            typelist << ( type.isEmpty() ? it : type );
        }
    }
    return result.insert(1,typelist.join(','));
}

QString Editor1::scopeOf(Stream &stream) const
{
    QString scope;
    if(stream.next(Stream::ScopeOperator))
        scope = stream.value();

    QString type;
    while( ! (type = typeOfNextSection(stream,scope)).isEmpty() )
    {
        scope = type;
        if(stream.next(Stream::Dereference))
        {
            if( !scope.endsWith('*'))
                return QString();
            scope.chop(1);
            scope.append("::");
        }
        else if(stream.next(Stream::Reference))
            scope.append("::");
        else
            break;
    }
    return stream.hasNext() ?  QString() : scope;
}

QString Editor1::completePrefix(const QTextCursor &tc) const
{
    //include.
    QTextCursor tmp = tc;
    tmp.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
    QRegExp rx("^\\s*#include\\s*[\"<]\\s*([a-zA-Z_.0-9]*)$");
    if(tmp.selectedText().contains(rx) )
        return "#include::" + rx.cap(1);

    Stream stream = rtake(tc);
    if(stream.isEmpty())
        return QString();

    QString prefix;
    if(stream.rflag(-1) == Stream::Word)
    {
        QChar ch = document()->characterAt(tc.position()-1);
        if( ch.isSpace() )
            return QString();

        prefix = stream.rdata(-1);
        stream.setend(-2);
    }

    QString scope = scopeOf(stream);
    if(scope.endsWith("::"))
        prefix.prepend(scope);
    return prefix;
}

QTextCursor Editor1::findPair(QChar to,const QTextCursor &from,QTextCursor::MoveOperation op) const
{
    assert(!from.isNull());
    assert(op == QTextCursor::PreviousCharacter || op == QTextCursor::NextCharacter);
    QChar fch = document()->characterAt(from.position());

    QTextCursor tc = from;
    int numOpen = 1;
    while( (numOpen != 0) && tc.movePosition(op))
    {
        if(document()->characterAt(tc.position()) == fch && tc.charFormat() != highlighter->NCCFormat)
            ++numOpen;
        else if(document()->characterAt(tc.position()) == to && tc.charFormat() != highlighter->NCCFormat)
            --numOpen;
    }
    return tc;
}

void Editor1::formattedInsertText(const QString text,QTextCursor &tc)
{
    QString indent = indentationOf(tc);
    QString modifiedText;
    QTextStream out((QString *)(&text),QIODevice::ReadOnly);
    while (!out.atEnd())
        modifiedText.append(indent).append(out.readLine()).append('\n');
    modifiedText.chop(1);
    tc.insertText(modifiedText);
}

QString Editor1::indentationOf(QTextCursor tc,bool indentable)
{
    while( (!(tc=find(QRegExp("[{}]"),tc,QTextDocument::FindBackward)).isNull()) && tc.selectedText() == QLatin1String("}"))
    {
        if(!tc.movePosition(QTextCursor::PreviousCharacter))
            return QString();
        assert(QLatin1Char('}') == document()->characterAt(tc.position()));
        tc = findPair(QLatin1Char('{'),tc,QTextCursor::PreviousCharacter);
        if(tc.isNull())
            return QString();
    }

    if(tc.isNull()) return QString();

    tc.select(QTextCursor::LineUnderCursor);
    QRegExp rx("^\\s*");
    rx.lastIndexIn(tc.selectedText());
    QString indent = rx.cap();
    indent.remove(QChar::Separator_Paragraph);
    if(indentable) indent.append(indentspace);
    return indent;
}

void Editor1::updateEnv()
{
    updateEnvModel();
    highlighter->rehighlight();
}

void Editor1::keyPressEvent(QKeyEvent *e)
{
    if (tmc->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    //new line.
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        QTextCursor tc = textCursor();
        QString indent = indentationOf(tc);
        tc.insertBlock();
        tc.insertText(indent);
        e->accept();
        return;
    }

    if (e->key() == Qt::Key_Tab /*&& e->modifiers() != Qt::ShiftModifier*/)
    {
        QTextCursor tc = textCursor();
        tc.clearSelection();
        tc.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
        QString text = tc.selectedText();
        int pos = 0;
        if(text.trimmed().isEmpty())
            pos = text.length() % indentspace.length();
        textCursor().insertText(indentspace.mid(pos) );
        e->accept();
        return;
    }

    //":"
    if(e->text() == QLatin1String(":"))
    {
        QTextCursor tc = textCursor();
        tc.movePosition(QTextCursor::PreviousWord,QTextCursor::KeepAnchor);
        QString prewd = tc.selectedText();
        tc.clearSelection();
        tc.movePosition(QTextCursor::StartOfLine,QTextCursor::KeepAnchor);
        QString str = tc.selectedText().trimmed();
        if(prewd.contains(QRegExp("^public\\s*$|^private\\s*$|^protected\\s*$")) && str.isEmpty() )
        {
            tc.removeSelectedText();
            QString space = indentationOf(tc,false);
            space.chop(indentspace.length());
            tc.insertText( space);
        }
    }

    //"}"
    if(e->text() == QLatin1String("}"))
    {
        QTextCursor tc = textCursor();
        tc.select(QTextCursor::LineUnderCursor);
        if(tc.selectedText().trimmed().isEmpty())
        {
            tc.removeSelectedText();
            tc.insertText( indentationOf(tc,false));
        }
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    //--------------autocompleter---------------
    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!isShortcut) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (ctrlOrShift && e->text().isEmpty())
        return;

    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = completePrefix(textCursor());

    if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length()<3 || completionPrefix.endsWith(')') || completionPrefix.endsWith(']')) )
    {
        tmc->popup()->hide();
        return;
    }

    if (completionPrefix != tmc->completionPrefix())
    {
        tmc->setCompletionPrefix(completionPrefix);
        tmc->popup()->setCurrentIndex(tmc->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(tmc->popup()->sizeHintForColumn(0)
                + tmc->popup()->verticalScrollBar()->sizeHint().width());
    tmc->complete(cr); // popup it up!
}

QString Editor1::tooltip(QTextCursor tc) const
{
    tc.select(QTextCursor::WordUnderCursor);
    QString st = tc.selectedText();

    if(! st.contains(QRegExp("^[a-zA-Z_]\\w*$")))
        return QString();

    tc.setPosition(tc.anchor());
    Stream stream(rtake(tc));
    QStandardItem * item = objectModel->itemOf(st,scopeOf(stream) );

    return item ? item->data(Qt::ToolTipRole).toString() : QString();
}

void Editor1::mouseDoubleClickEvent(QMouseEvent * e)
{
    QPlainTextEdit::mouseDoubleClickEvent(e);
    QTextCursor tc = textCursor();
    if(! tc.hasSelection())
        return;
    int pos = tc.position();
    int anchor = tc.anchor();

    if(      document()->characterAt(anchor-2) == QLatin1Char('/') &&
             document()->characterAt(anchor-1) == QLatin1Char('*') &&
             document()->characterAt(pos) == QLatin1Char('*') &&
             document()->characterAt(pos+1) == QLatin1Char('/') )
    {
        tc.setPosition(anchor-2);
        tc.setPosition(pos+2,QTextCursor::KeepAnchor);
        setTextCursor(tc);
    }
}

void Editor1::wheelEvent(QWheelEvent* e)
{
    QFont f = this->font();
    if ((e->modifiers() == Qt::ControlModifier) && (e->delta() > 0))
    {
        f.setPointSizeF(f.pointSizeF()+0.5);
        this->setFont(f);
        highlighter->rehighlight();
    }
    else if ((e->modifiers() == Qt::ControlModifier) && (e->delta() < 0) && (f.pointSizeF() > 0.5))
    {
        f.setPointSizeF(f.pointSizeF()-0.5);
        this->setFont(f);
        highlighter->rehighlight();
    }
    else
        QPlainTextEdit::wheelEvent(e);
}

bool Editor1::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        const QPoint point = this->viewport()->mapFromParent(helpEvent->pos());
        QTextCursor tc = cursorForPosition(point);
        QString tips = tooltip(tc);

        if(! tips.isEmpty())
            QToolTip::showText(helpEvent->globalPos(),tips);
        else
        {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
    return QPlainTextEdit::event(event);
}

void Editor1::focusInEvent(QFocusEvent *e)
{
    tmc->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}
