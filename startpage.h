/** @file startpage.h
* @brief the start page.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the start page (the opening page of the soft).
*/

#ifndef STARTPAGE_H
#define STARTPAGE_H

#include <QSplitter>
#include <QMap>

QT_BEGIN_NAMESPACE
class QStackedWidget;
QT_END_NAMESPACE
class Projist;
class Codist;
class RichTextWidget;
class CodeEditor;
class ProjectEditor;

class StartPage : public QSplitter
{
    Q_OBJECT
public:
    explicit StartPage(QWidget *parent = 0);

    bool isWindowModified() const;
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void displayCode(const QString &codePath);
    void displayProject(const QString &projectPath);
    void renameCode(const QString &path,const QString &oldName, const QString &newName);
    void renameProject(const QString &path,const QString &oldName, const QString &newName);
    void updateEditorEnvironment();
private:
    QStackedWidget *stackedWidget;
    RichTextWidget *welcomeWidget;    
    QMap<QString,ProjectEditor *> mapproj;
    QMap<QString,CodeEditor *> mapcode;
public:
    Projist *projist;
    Codist *codist;
};

#endif // STARTPAGE_H
