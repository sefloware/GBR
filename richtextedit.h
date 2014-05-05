/** @file richtextedit.h
* @brief the rich text editor.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the rich text editor Class
* which is derived from the QFile Class.
*/

#ifndef RICHTEXTEDIT_H
#define RICHTEXTEDIT_H

#include <QImageReader>
#include <QFileInfo>
#include <QTextEdit>
#include <QMimeData>
#include <QUrl>
#include <QDir>

class RichTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit RichTextEdit(QWidget *parent = 0);
    void setWindowFilePath(const QString &filePath);
    bool saveImages();
    void initializeResource();
    void setHtml(const QString &text);
public slots:
    void insertImage();
protected:
    bool canInsertFromMimeData(const QMimeData* source) const;
    void insertFromMimeData(const QMimeData* source);
private:
    QString uniqueFileName(const QString &filePath);
    bool clearOldImages();
    void dropImage(const QUrl& name, const QImage& image);
    void dropTextFile(const QString &filePath);
private:
    QFileInfo imageFolderInfo;

    enum {MaxBaseNameLength = 20};
};

#endif // RICHTEXTEDIT_H
