/** @file mainwindow.h
* @brief the main window
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the main window Class.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QPushButton;
class QTabWidget;
QT_END_NAMESPACE
class StartPage;
class cppkeywords;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void openProject(const QString &path);
    void updateResultTable(const QString &path);
    bool closeProject(int index);
    void updateEditorEnvironment();
private:
    QTabWidget *tabWidget;
    StartPage *startPage;
};

#endif // MAINWINDOW_H
