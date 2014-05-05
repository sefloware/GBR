/** @file richtextwidget.h
* @brief the rich text widget.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the rich text widget which
* composed of the rich text editor and some
* edit buttons.
*/

#ifndef RICHTEXTWIDGET_H
#define RICHTEXTWIDGET_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include "startpage.h"

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
QT_END_NAMESPACE
class RichTextEdit;

class RichTextWidget : public QMainWindow
{
    Q_OBJECT

public:
    RichTextWidget(QWidget *parent = 0);

    bool fileOpen(const QString &fn);
    void setWindowFilePath(const QString &filePath);
    void setReadOnly(bool ro);
    bool maybeSave();
signals:
    void modificationChanged(bool);
protected:
    virtual void closeEvent(QCloseEvent *e);
private:
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    bool load(const QString &f);
public slots:
    bool fileSave();
private slots:
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void printPreview(QPrinter *);
private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionInsertPicture;


    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    QToolBar *tb;
    QString fileName;
    RichTextEdit *textEdit;
};

#endif // RICHTEXTWIDGET_H
