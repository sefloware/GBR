/** @file richtextedit.cpp
* @brief the rich text editor.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the rich text editor Class
* which is derived from the QFile Class.
*/

#include "richtextedit.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QVariant>
#include <QToolTip>
#include <QTextBlock>

RichTextEdit::RichTextEdit(QWidget *parent) :
    QTextEdit(parent)
{}

void RichTextEdit::setWindowFilePath(const QString &filePath)
{
    this->setEnabled(true);

    QTextEdit::setWindowFilePath(filePath);
    const QFileInfo info(filePath);
    if(! info.isNativePath())
        return;

    const QString folderName = info.completeBaseName()+".files";
    imageFolderInfo = QFileInfo(info.absoluteDir().absoluteFilePath(folderName));


   if(! imageFolderInfo.exists() && ! imageFolderInfo.absoluteDir().mkdir(folderName))
   {
       this->setEnabled(false);
       QMessageBox::warning(this,tr("Warning"),"Failed to create the folder <b>" + folderName + "</b>.",QMessageBox::Ok,QMessageBox::Ok);
   }
   else if(!  imageFolderInfo.isDir() )
   {
       this->setEnabled(false);
       QMessageBox::warning(this,tr("Warning"),"Failed to create the folder <b>" + folderName + "</b> whose name is taken.",QMessageBox::Ok,QMessageBox::Ok);
   }
}

bool RichTextEdit::canInsertFromMimeData(const QMimeData* source) const
{
    return source->hasImage() || source->hasUrls() || QTextEdit::canInsertFromMimeData(source);
}

QString RichTextEdit::uniqueFileName(const QString &filePath)
{
    QFileInfo info(filePath);
    QString baseName = info.completeBaseName();
    if( baseName.length() > MaxBaseNameLength)
        baseName.truncate(MaxBaseNameLength);


    QDir dir(imageFolderInfo.absoluteFilePath());
    QString fileName = baseName  + "." + info.suffix();
    int count = 0;
    while (dir.exists(fileName))
    {
        ++count;
        fileName = baseName + QString::number(count)  + "." + info.suffix();
    }
    return fileName;
}

void RichTextEdit::insertFromMimeData(const QMimeData* source)
{
    if (source->hasImage())
        foreach (const QUrl &it, source->urls())
        {
            QUrl name = QUrl(imageFolderInfo.fileName() + "/" + uniqueFileName(it.toLocalFile())) ;
            dropImage(name, qvariant_cast<QImage>(source->imageData()));
        }
    else if (source->hasUrls())
        foreach (QUrl it, source->urls())
        {
            QFileInfo info(it.toLocalFile());
            if (QImageReader::supportedImageFormats().contains(info.suffix().toLower().toLatin1()))
            {
                QUrl name = QUrl(imageFolderInfo.fileName() + "/" + uniqueFileName(it.toLocalFile())) ;
                dropImage(name, QImage(info.filePath()));
            }
            else
                dropTextFile(info.filePath());
        }
    else
        QTextEdit::insertFromMimeData(source);
}

bool RichTextEdit::clearOldImages()
{
    QDir dir(imageFolderInfo.absoluteFilePath() );

    QStringList list = dir.entryList(QDir::Files);

    foreach (const QString &it, list) {
        if(! dir.remove(it))
            return false;
    }

    return true;
}

void RichTextEdit::setHtml(const QString &text)
{
    QTextEdit::setHtml(text);
    initializeResource();
}

void RichTextEdit::initializeResource()
{
    QStringList images;
    QTextBlock b = document()->begin();
    while (b.isValid())
    {
        for (QTextBlock::iterator i = b.begin(); !i.atEnd(); ++i)
        {
            QTextCharFormat format = i.fragment().charFormat();
            bool isImage = format.isImageFormat();
            if (isImage)
                images << format.toImageFormat().name();
        }
        b = b.next();
    }

    foreach (const QString &it, images)
    {
        const QString fn = imageFolderInfo.absoluteDir().absoluteFilePath(it);
        QImage image = QImageReader(fn).read();
        if(! image.isNull())
            document()->addResource(QTextDocument::ImageResource,QUrl(it), image);
    }
}

bool RichTextEdit::saveImages()
{
    if(! clearOldImages())
    {
        QMessageBox::critical(this,tr("Error"),"Failed to clear the old images in folder:\n"+imageFolderInfo.absoluteFilePath(),QMessageBox::Ok,QMessageBox::Ok);
        return false;
    }

    QStringList images;
    QTextBlock b = document()->begin();
    while (b.isValid())
    {
        for (QTextBlock::iterator i = b.begin(); !i.atEnd(); ++i)
        {
            QTextCharFormat format = i.fragment().charFormat();
            bool isImage = format.isImageFormat();
            if (isImage)
                images << format.toImageFormat().name();
        }
        b = b.next();
    }

    foreach (const QString &it, images)
    {
        const QString fn = imageFolderInfo.absoluteDir().absoluteFilePath(it);

        QVariant valimg = document()->resource(QTextDocument::ImageResource, it);
        switch (valimg.type())
        {
        case QVariant::Image:
            if(! valimg.value<QImage>().save(fn))
            {
                QMessageBox::critical(this,tr("Error"),"Failed to save the images in the rich text.",QMessageBox::Ok,QMessageBox::Ok);
                return false;
            }
            break;
        case QVariant::Pixmap:
            if(! valimg.value<QPixmap>().save(fn))
            {
                QMessageBox::critical(this,tr("Error"),"Failed to save the images in the rich text.",QMessageBox::Ok,QMessageBox::Ok);
                return false;
            }
            break;
        default:
            break;
        }
    }

    return true;
}

void RichTextEdit::dropImage(const QUrl& name, const QImage& image)
{
    if (!image.isNull())
    {
        document()->addResource(QTextDocument::ImageResource, name, image);
        textCursor().insertImage(name.toString());
    }
}

void RichTextEdit::dropTextFile(const QString& filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        textCursor().insertText(file.readAll());
}

void RichTextEdit::insertImage()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select an image"),
                                                ".", tr("JPEG (*.jpg *jpeg)\n"
                                                        "Bitmap Files (*.bmp)\n"
                                                        "GIF (*.gif)\n"
                                                        "PNG (*.png)\n"));
    static int i = 0;
    QString fn = QString::number(++i).prepend('i').append('.').append(QFileInfo(file).suffix());
    QUrl url = QUrl(imageFolderInfo.fileName() + "/" + fn) ;
    QImage image = QImageReader (file).read();
    if(image.isNull())
        return;
    dropImage(url, image );
}
