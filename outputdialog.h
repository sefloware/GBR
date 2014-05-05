/** @file outputdialog.h
* @brief the output dialog.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the output dialog Class,
* which provides a simple convenience dialog
* to get a output path from the user.
*/

#ifndef OUTPUTDIALOG_H
#define OUTPUTDIALOG_H

#include <QDialog>
#include <QDir>

QT_BEGIN_NAMESPACE
class QValidator;
QT_END_NAMESPACE
namespace Ui {
class OutputDialog;
}

class OutputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OutputDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~OutputDialog();
    void initializePath();
    void setNameValidator(const QValidator *nameValidator);
    QString getPath();

    static QString defaultPath();
private slots:
    void textChanged();
    void exploreDirectory();
    void savePath();
private:
    Ui::OutputDialog *ui;
};

#endif // OUTPUTDIALOG_H
