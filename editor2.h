/** @file editor2.h
* @brief the C++ editor2.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the the C++ editor Class inherited
* from the Editor1 Class.
*/

#ifndef EDITOR2_H
#define EDITOR2_H

#include "editor1.h"
#include <QObject>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE
class LineNumberArea;

class Editor2 : public Editor1
{
    Q_OBJECT

public:
    Editor2(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    LineNumberArea *lineNumberArea;
    int currentBlockNumber;
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(Editor2 *editor) : QWidget(editor) {
        this->setObjectName("LineNumberArea");
        codeEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        QWidget::paintEvent(event);
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    Editor2 *codeEditor;
};

#endif //EDITOR2_H
