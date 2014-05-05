/** @file richtextwidget.cpp
* @brief the rich text widget.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the rich text widget which
* composed of the rich text editor and some
* edit buttons.
*/

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMimeData>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#endif
#include "richtextwidget.h"
#include "richtextedit.h"
#include "assert.h"

const QString rsrcPath = ":/icon2/images2";

RichTextWidget::RichTextWidget(QWidget *parent)
    : QMainWindow(parent)
{
    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupTextActions();

    textEdit = new RichTextEdit(this);
    connect(textEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this, SLOT(currentCharFormatChanged(QTextCharFormat)));
    connect(textEdit, SIGNAL(cursorPositionChanged()),
            this, SLOT(cursorPositionChanged()));

    setCentralWidget(textEdit);

    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    connect(textEdit->document(), SIGNAL(modificationChanged(bool)),
            actionSave, SLOT(setEnabled(bool)));
    connect(textEdit->document(), SIGNAL(modificationChanged(bool)),
            this, SLOT(setWindowModified(bool)));
    connect(textEdit->document(), SIGNAL(modificationChanged(bool)),
            this, SIGNAL(modificationChanged(bool)));
    connect(textEdit->document(), SIGNAL(undoAvailable(bool)),
            actionUndo, SLOT(setEnabled(bool)));
    connect(textEdit->document(), SIGNAL(redoAvailable(bool)),
            actionRedo, SLOT(setEnabled(bool)));

    setWindowModified(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isModified());
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

    connect(actionUndo, SIGNAL(triggered()), textEdit, SLOT(undo()));
    connect(actionRedo, SIGNAL(triggered()), textEdit, SLOT(redo()));

    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);

    connect(actionCut, SIGNAL(triggered()), textEdit, SLOT(cut()));
    connect(actionCopy, SIGNAL(triggered()), textEdit, SLOT(copy()));
    connect(actionPaste, SIGNAL(triggered()), textEdit, SLOT(paste()));
    connect(actionInsertPicture, SIGNAL(triggered()), textEdit, SLOT(insertImage()) );

    connect(textEdit, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));

#ifndef QT_NO_CLIPBOARD
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
#endif
    setReadOnly(true);
}

void RichTextWidget::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}

void RichTextWidget::setupFileActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("File Actions"));
    tb->setMovable(false);
    addToolBar(tb);

    QAction *a;

    actionSave = a = new QAction(QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png")),
                                 tr("&Save"), this);
    a->setShortcut(QKeySequence::Save);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
    a->setEnabled(false);
    tb->addAction(a);

#ifndef QT_NO_PRINTER
    a = new QAction(QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png")),
                    tr("&Print..."), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
    tb->addAction(a);

    a = new QAction(QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png")),
                    tr("Print Preview..."), this);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPreview()));
    tb->addAction(a);

    a = new QAction(QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png")),
                    tr("&Export PDF..."), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPdf()));
    tb->addAction(a);

#endif
}

void RichTextWidget::setupEditActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Edit Actions"));
    tb->setMovable(false);
    addToolBar(tb);

    QAction *a;
    a = actionUndo = new QAction(QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png")),
                                              tr("&Undo"), this);
    a->setShortcut(QKeySequence::Undo);
    tb->addAction(a);
    a = actionRedo = new QAction(QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png")),
                                              tr("&Redo"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Redo);
    tb->addAction(a);
    a = actionCut = new QAction(QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png")),
                                             tr("Cu&t"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Cut);
    tb->addAction(a);
    a = actionCopy = new QAction(QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png")),
                                 tr("&Copy"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Copy);
    tb->addAction(a);
    a = actionPaste = new QAction(QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png")),
                                  tr("&Paste"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Paste);
    tb->addAction(a);
    a = actionInsertPicture = new QAction(QIcon::fromTheme("insert-picture", QIcon(rsrcPath + "/picture.png")),
                                  tr("&Insert Picture"), this);
    a->setPriority(QAction::LowPriority);
    tb->addAction(a);
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void RichTextWidget::setupTextActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Format Actions"));
    tb->setMovable(false);
    addToolBar(tb);

    actionTextBold = new QAction(QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/textbold.png")),
                                 tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
	QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon::fromTheme("format-text-italic",
                                                    QIcon(rsrcPath + "/textitalic.png")),
                                   tr("&Italic"), this);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon::fromTheme("format-text-underline",
                                                       QIcon(rsrcPath + "/textunder.png")),
                                      tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction*)), this, SLOT(textAlign(QAction*)));

    // Make sure the alignLeft  is always left of the alignRight
    if (QApplication::isLeftToRight()) {
        actionAlignLeft = new QAction(QIcon::fromTheme("format-justify-left",
                                                       QIcon(rsrcPath + "/textleft.png")),
                                      tr("&Left"), grp);
        actionAlignCenter = new QAction(QIcon::fromTheme("format-justify-center",
                                                         QIcon(rsrcPath + "/textcenter.png")),
                                        tr("C&enter"), grp);
        actionAlignRight = new QAction(QIcon::fromTheme("format-justify-right",
                                                        QIcon(rsrcPath + "/textright.png")),
                                       tr("&Right"), grp);
    } else {
        actionAlignRight = new QAction(QIcon::fromTheme("format-justify-right",
                                                        QIcon(rsrcPath + "/textright.png")),
                                       tr("&Right"), grp);
        actionAlignCenter = new QAction(QIcon::fromTheme("format-justify-center",
                                                         QIcon(rsrcPath + "/textcenter.png")),
                                        tr("C&enter"), grp);
        actionAlignLeft = new QAction(QIcon::fromTheme("format-justify-left",
                                                       QIcon(rsrcPath + "/textleft.png")),
                                      tr("&Left"), grp);
    }
    actionAlignJustify = new QAction(QIcon::fromTheme("format-justify-fill",
                                                      QIcon(rsrcPath + "/textjustify.png")),
                                     tr("&Justify"), grp);

    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    tb->addActions(grp->actions());

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("&Color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));
    tb->addAction(actionTextColor);

    tb = new QToolBar(this);
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    tb->setWindowTitle(tr("Format Actions"));
    tb->setMovable(false);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    comboStyle = new QComboBox(tb);
    tb->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    comboStyle->addItem("Ordered List (Roman lower)");
    comboStyle->addItem("Ordered List (Roman upper)");
    connect(comboStyle, SIGNAL(activated(int)), this, SLOT(textStyle(int)));

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, SIGNAL(activated(QString)), this, SLOT(textFamily(QString)));

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    QFontDatabase db;
    foreach(int size, db.standardSizes())
        comboSize->addItem(QString::number(size));

    connect(comboSize, SIGNAL(activated(QString)), this, SLOT(textSize(QString)));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font()
                                                                   .pointSize())));
}

bool RichTextWidget::load(const QString &f)
{
    QFile file(f);
    if (! file.exists())
    {
        textEdit->clear();
        return true;
    }

    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        textEdit->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        textEdit->setPlainText(str);
    }

    return true;
}

bool RichTextWidget::maybeSave()
{
    if (fileName.startsWith(QLatin1String(":/")))
        return true;
    if (!textEdit->document()->isModified())
        return true;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Unsaved Changes Warning"), tr("The document has been changed."), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return fileSave();
    case QMessageBox::Discard:
        return load(textEdit->windowFilePath());
    case QMessageBox::Cancel:
        textEdit->setFocus();
        return false;
    default:
        return true;
    }
    return true;
}

void RichTextWidget::setWindowFilePath(const QString &filePath)
{
    textEdit->setWindowFilePath(filePath);
    QMainWindow::setWindowFilePath(filePath);
}

bool RichTextWidget::fileOpen(const QString &fn)
{
    this->setWindowFilePath(fn);

    assert(! fn.isEmpty());

    fileName = fn;
    textEdit->document()->setModified(false);
    return load(fn);
}

bool RichTextWidget::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();

    if(! textEdit->saveImages())
        return false;

    QTextDocumentWriter writer(fileName);
    bool success = writer.write(textEdit->document());
    if (success)
        textEdit->document()->setModified(false);
    else
    {
        QMessageBox::critical(this,tr("Error"), tr("Failed to save the file."),QMessageBox::Ok,QMessageBox::Ok);
        return fileSaveAs();
    }

    return success;
}

bool RichTextWidget::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as ..."), QString(),
                                              tr("HTML-Files (*.htm *.html);;"
                                                 "ODF files (*.odt);;All Files (*)"));
    if (fn.isEmpty())
        return false;
    if (!(fn.endsWith(".odt", Qt::CaseInsensitive)
          || fn.endsWith(".htm", Qt::CaseInsensitive)
          || fn.endsWith(".html", Qt::CaseInsensitive))) {
        fn += ".html"; // default
    }
    fileName = fn;
    textEdit->document()->setModified(false);
    return fileSave();
}

void RichTextWidget::setReadOnly(bool ro)
{
    if(! isEnabled())
        return;

    QList<QToolBar *> toolbars = this->findChildren<QToolBar *>();
    foreach (QToolBar *it, toolbars)
        it->setHidden(ro);

    textEdit->setReadOnly(ro);
    textEdit->viewport()->setAutoFillBackground(! ro);
    if(!ro) textEdit->setFocus();
}

void RichTextWidget::filePrint()
{
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (textEdit->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
        textEdit->print(&printer);
    delete dlg;
#endif
}

void RichTextWidget::filePrintPreview()
{
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreview(QPrinter*)));
    preview.exec();
#endif
}

void RichTextWidget::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    textEdit->print(printer);
#endif
}

void RichTextWidget::filePrintPdf()
{
#ifndef QT_NO_PRINTER
//! [0]
    QString fileName = QFileDialog::getSaveFileName(this, "Export PDF",
                                                    QString(), "*.pdf");
    if (!fileName.isEmpty()) {
        if (QFileInfo(fileName).suffix().isEmpty())
            fileName.append(".pdf");
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        textEdit->document()->print(&printer);
    }
//! [0]
#endif
}

void RichTextWidget::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void RichTextWidget::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void RichTextWidget::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void RichTextWidget::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void RichTextWidget::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void RichTextWidget::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();

    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
            case 7:
                style = QTextListFormat::ListLowerRoman;
                break;
            case 8:
                style = QTextListFormat::ListUpperRoman;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void RichTextWidget::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void RichTextWidget::textAlign(QAction *a)
{
    if (a == actionAlignLeft)
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
}

void RichTextWidget::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

void RichTextWidget::cursorPositionChanged()
{
    alignmentChanged(textEdit->alignment());
}

void RichTextWidget::clipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void RichTextWidget::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}

void RichTextWidget::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void RichTextWidget::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void RichTextWidget::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);
}
