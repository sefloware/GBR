/** @file editor2.cpp
* @brief the C++ editor2.
* @author Runhua Li
* @date 2013.6
* @version v1.0
* @note
* The file is about the the C++ editor Class inherited
* from the Editor1 Class.
*/

#include <QtWidgets>
#include "editor2.h"

Editor2::Editor2(QWidget *parent) :
    Editor1(parent), currentBlockNumber(-1)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
}

int Editor2::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 12 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void Editor2::highlightCurrentLine()
{
    currentBlockNumber = textCursor().blockNumber();
}

void Editor2::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor2::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void Editor2::resizeEvent(QResizeEvent *e)
{
    Editor1::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor2::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.save();
            QFont f= this->font();
            if(blockNumber == currentBlockNumber)
            {
                painter.setPen(Qt::black);
                f.setWeight(QFont::DemiBold);
            }
            painter.setFont(f);
            painter.drawText(0, top, lineNumberArea->width()-4, fontMetrics().height(),
                             Qt::AlignRight, number);
            painter.restore();
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
