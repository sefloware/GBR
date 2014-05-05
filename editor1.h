/** @file editor1.h
* @brief the C++ editor1.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the the C++ editor Class inherited
* from the QPlainTextEdit Class.
*/

#ifndef EDITOR1_H
#define EDITOR1_H

#include <QPlainTextEdit>
#include <QRegExp>
#include "editormap.h"
#include "assert.h"

QT_BEGIN_NAMESPACE
class QSortFilterProxyModel;
QT_END_NAMESPACE
class CppObjectModel;
class Highlighter;
class CodeCompleter;
class CppPosQuery;
class Stream;

class Editor1 : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Editor1(QWidget *parent = 0);
    CppObjectModel *cppObjectModel() const
    { return objectModel; }
    QString indentationOf(QTextCursor tc, bool indentable = true);
    void formattedInsertText(const QString text,QTextCursor &tc);
    virtual void updateEnv();
protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
    bool event(QEvent *event); //tool tips.
    void mouseDoubleClickEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent* e);
public slots:
    void updateEnvModel();
private slots:
    void insertCompletion(const QModelIndex &index);
    void mark();
private:
    //prepare for autocompleter, highlighter and tooltips.
    QString rtakeOne(QTextCursor &tc) const; //get one useful word backward, such as '->', '.', 'word::word::', 'word', '[.]', '(.)', '(.,'
    Stream rtake(QTextCursor tc) const; //get the whole stream constructed of the useful word backward.

    //prepare for getting types of arguments.
    QString takeOne(int &pos, const CppPosQuery &posquery) const; //get one useful word from the posquery, such as '->', '.', 'word::word::', 'word', '[.]', '(.)'
    Stream take(QString str) const; //get the whole stream constructed of the useful word backward.

    //parsing.
    inline QString argtypes(QString arg) const;
    QString typeOfNextSection(Stream &stream, const QString &scope) const;
    QString typeOfLocar(const QString &var) const; // type of local variable.
    QString scopeOf(Stream &stream) const; //to get the type of the stream. stream should end with '->', '.', '::'.

    //result.
    QString tooltip(QTextCursor tc) const; // for tool tips.
    QString completePrefix(const QTextCursor &tc) const; //for autocompleter. stream should end with '->', '.', '::', word.

public:
    QTextCursor findPair(QChar to,const QTextCursor &from,QTextCursor::MoveOperation op = QTextCursor::NextCharacter) const;

    QTextCursor find(const QString &subString, int from = 0, QTextDocument::FindFlags options = 0) const;
    QTextCursor find(const QString &subString, const QTextCursor &from, QTextDocument::FindFlags options = 0) const;
    QTextCursor find(const QRegExp &expr, int from = 0, QTextDocument::FindFlags options = 0) const;
    QTextCursor find(const QRegExp &expr, const QTextCursor &from, QTextDocument::FindFlags options = 0) const;
public:
    static CppObjectModelMap &source();
    CppObjectModel *objectModel;
private:
    friend class Highlighter;
    CodeCompleter *tmc;
    Highlighter *highlighter;

    const QString indentspace;
};

class Stream
{
public:
    Stream() : pos(-1),end(-1) {}
    enum ItemFlags{ NoFlag = 0,
                    Word = 1, // 'word'
                    SquareOperator = 2, // '[]'
                    RoundOperator = 4, // '()'
                    Reference = 8, // '.'
                    Dereference = 16, // '->'
                    ScopeOperator = 32 // 'word::'
                    };
    bool next(int flag)
    {
        if(pos >= flags.count()+end || ! (flags[pos+1] & flag) )
            return false;
        ++pos;
        return true;
    }
    bool hasNext() const
    {
        return pos < flags.count() + end;
    }
    const QString &value() const
    { return datas.at(pos); }
    ItemFlags flag() const
    { return flags.at(pos); }
    int postion() const
    { return pos;}

    void reset()
    { pos = -1; end = -1;}
    void setpos(int pos)
    {
        assert(pos > -1);
        this->pos = pos;
    }
    void setend(int end)
    {
        assert(end < 0);
        this->end = end;
    }

    bool isEmpty() const
    { return datas.isEmpty(); }

    ItemFlags flag(int i) const
    {  return flags.at(i);  }
    ItemFlags rflag(int i) const
    {
        i += flags.count();
        return flags.at(i);
    }
    const QString &data(int i) const
    {  return datas.at(i); }
    const QString &rdata(int i) const
    {
        i += datas.count();
        return datas.at(i);
    }

    static ItemFlags flag(const QString &word)
    {
        if (word.contains(QRegExp("^[a-zA-Z_]\\w*$")))
            return Word;
        else if (word.startsWith('(') && word.endsWith(')'))
            return RoundOperator;
        else if (word.startsWith('[') && word.endsWith(']'))
            return SquareOperator;
        else if (word == QLatin1String("."))
            return Reference;
        else if (word == QLatin1String("->"))
            return Dereference;
        else if(word == QLatin1String("::") )
            return ScopeOperator;
        else
            return NoFlag;
    }

    void append(const QString &str,ItemFlags flag)
    { datas.append(str); flags.append(flag); }
    void prepend(const QString &str,ItemFlags flag)
    { datas.prepend(str); flags.prepend(flag); }
private:
    int pos;
    int end;
    QStringList datas;
    QList<ItemFlags> flags;
};

class VisibleRegion
{
public:
    VisibleRegion( QTextCursor from,const Editor1 *editor)
    {
        while( ! (from = editor->find("}",from,QTextDocument::FindBackward) ).isNull() )
        {
            from.setPosition(from.anchor());
            locs.push_back(from.position());
            if( (from = editor->findPair('{',from,QTextCursor::PreviousCharacter)).isNull())
                return;
            else
                locs.push_back(from.position());
        }
    }
    bool operator()(const QTextCursor &tc) const
    {
        return operator()(tc.position());
    }
    bool operator() (int position) const
    {
        for (int i = 0; i < locs.size(); ++i)
            if(locs[i] < position)
                return !(i%2);
        return true;
    }
private:
    QVector<int> locs;
};

#endif // EDITOR1_H
