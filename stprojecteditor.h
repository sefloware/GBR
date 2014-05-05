/** @file stprojecteditor.h
* @brief the project description editor in the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the project description editor which
* is composed of the edit bar and the RichTextWidget object.
*/

#ifndef STPROJECTEDITOR_H
#define STPROJECTEDITOR_H

#include<QWidget>
QT_BEGIN_NAMESPACE
class QLabel;
class QToolButton;
class QToolBar;
//class QWebView;
//class QStackedLayout;
QT_END_NAMESPACE
class RichTextWidget;
class Projist;

class ProjectEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectEditor(const QString &projectpath, Projist *projist, QWidget *parent = 0);
    bool save();
    void setWindowFilePath(const QString &projectPath);
private slots:
    void setReadWrite(bool rw);
    void setModification(bool modified);
private:
    QString descriptionFilePath(const QString &projectPath);
private:
    QLabel *labelTitle;
    QToolButton *switcher;
    QToolBar *bar;
    RichTextWidget *richTextWidget;

    Projist *projist;
};

#endif // STPROJECTEDITOR_H
