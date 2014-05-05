/** @file editorhighlighter.cpp
* @brief the C++ syntax highlighter.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the syntax highlighter Class.
*/

#include "editorhighlighter.h"
#include "cppposquery.h"
#include "editor1.h"
#include "editorobjmodel.h"
#include "assert.h"
#include "cppobjlist.h"
#include <QTextStream>
#include <QRegExp>
#include <QDir>
#include <QStandardItem>

Highlighter::Highlighter(Editor1 *parent)
    : QSyntaxHighlighter(parent->document()),
      tc(parent->document()),
      editorCpp(parent)
{
    QFile file(QDir::toNativeSeparators("config/keywords.txt"));
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream out(&file);
        while(!out.atEnd())
            keys << out.readLine().split(QRegExp("\\s+"),QString::SkipEmptyParts);
        keys.removeDuplicates();
    }

    NCCFormat.setForeground(Qt::darkGreen);
    borderFormat.setForeground(Qt::blue);
    directiveFormat.setForeground(Qt::darkBlue);
}

void Highlighter::setFindString(const QString &findString)
{
    this->findString = findString;

    rehighlight();
}

void Highlighter::highlightBlock(const QString &text)
{
    setCurrentBlockState(previousBlockState());//-1.

    //text is empty.
    if(text.trimmed().isEmpty())
        return;

    if(previousBlockState() == -1 && text.trimmed().startsWith('#'))
        setFormat(0,text.length(),directiveFormat);

    int startIndex = 0;
    if (previousBlockState() == 1 && ( startIndex = text.indexOf("*/") + 2) == 1)
    {
        setFormat(0,text.length(),NCCFormat);
        return;
    }
    setCurrentBlockState(-1);


    CppPosQuery posQuery(text);
    for(int i=startIndex;i<text.length();)
    {
        const int anchor = i;
        switch(posQuery.flag(anchor))
        {
        case CppPosQuery::DoubleQuot:
        case CppPosQuery::SingleQuot:
        {
            while( posQuery. flag(++i) & ( CppPosQuery::SingleQuot | CppPosQuery::DoubleQuot) );
            setFormat(anchor, i-anchor, NCCFormat);
            break;
        }
        case CppPosQuery::SingleLineComment:
        {
            i=text.length();
            setFormat(anchor,i-anchor,NCCFormat);
            break;
        }
        case CppPosQuery::SingleExclamLineComment:
        {
            i=text.length();
            setFormat(anchor,i-anchor,borderFormat);
            break;
        }
        case CppPosQuery::MultiLineComment:
        {
            i = text.indexOf("*/",i) + 2;
            if(i==1)
            {
                i=text.length();
                setCurrentBlockState(1);
            }
            setFormat(anchor,i-anchor,NCCFormat);
            break;
        }
        default:
            ++i;
        }
    }

    assert(currentBlock().isValid());
    const int cbpos = currentBlock().position();
    int pos = 0;
    QRegExp rx("\\b[a-zA-Z_]\\w*\\b");
    while( (pos = rx.indexIn(text,pos)) != -1)
    {
        const QString word = rx.cap();
        const int length = rx.matchedLength();

        QTextCharFormat fmt = format(pos);
        if(     fmt == NCCFormat ||
                fmt == borderFormat ||
                fmt == directiveFormat )
        {
            pos += length;
            continue;
        }

        //---color the useful word----
        tc.setPosition(cbpos+pos);
        Stream stream(editorCpp->rtake(tc));
        QStandardItem * item = editorCpp->cppObjectModel()-> itemOf(word, editorCpp->scopeOf(stream));

        if (item)
        {
            QTextCharFormat _format;
            _format.setForeground(item->foreground());
            QFont f=item->font();
            f.setPointSizeF(editorCpp->fontInfo().pointSizeF());
            _format.setFont(f);
            if(word == findString)
                _format.setBackground(Qt::yellow);
            setFormat( pos, length, _format);
        }
        else if(keys.contains(word))
        {
            QTextCharFormat keyFormat;
            keyFormat.setForeground(Qt::magenta);
            if(word == findString)
                keyFormat.setBackground(Qt::yellow);
            setFormat( pos, length, keyFormat);
        }

        pos += length;
    }
}
