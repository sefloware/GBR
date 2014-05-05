/** @file editorhighlighter.h
* @brief the C++ syntax highlighter.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the syntax highlighter Class.
*/

#ifndef EDITORHIGHLIGHTER_H
#define EDITORHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextCursor>

class Editor1;

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(Editor1 *parent = 0);
    void setFindString(const QString &findString);
protected:
    void highlightBlock(const QString &text);
private:
    QTextCursor tc;
    Editor1 *editorCpp;

    QStringList keys;
    QString findString;
public:
    QTextCharFormat NCCFormat;
    QTextCharFormat borderFormat;
    QTextCharFormat directiveFormat;
};

#endif // EDITORHIGHLIGHTER_H
