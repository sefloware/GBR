/** @file mddebug.h
* @brief the model debuger.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the model debuger Class.
*/

#ifndef MDDEBUG_H
#define MDDEBUG_H

#include <QToolBar>
#include <QTextCharFormat>
#include <QProcess>

QT_BEGIN_NAMESPACE
class QDir;
class QLabel;
class QToolBar;
class QLineEdit;
class QComboBox;
class QToolButton;
class QPlainTextEdit;
class QDialogButtonBox;
QT_END_NAMESPACE

class MdDebug : public QToolBar
{
    Q_OBJECT

public:
    explicit MdDebug(QWidget *parent = 0);
    ~MdDebug();
    void run(const QStringList &commands);
    QModelIndex currentIndex() const;
    QDir workingDirectory() const;
    void setWorkingDirectory(const QString &dir);

    void setReadOnly(bool ro);
signals:
    void runTriggered();
    void exploreDebugFolder();
private slots:
    void setDebugMode(bool show);
    void readOutput();
    void handError(QProcess::ProcessError error);
    void slotFinished(int exitCode);

    void editRunEnv();
    void clearOutputWindow();
    void stopProcess();
    void runProcess();
public:
    QPlainTextEdit* textOutput;
private:
    QComboBox *box;
    QAction *settingAction;
    QAction *clearAction;
    QAction *stopAction;
    QAction *runAction;
    QAction *exploreAction;
    QToolButton *switcher;

    QProcess* process;
    QStringList commands;
    QTextCharFormat cmdformat;
    QTextCharFormat outformat;
    QTextCharFormat exitformat;
    QTextCharFormat errformat;
};

#endif // MDDEBUG_H
