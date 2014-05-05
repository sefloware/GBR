/** @file inputdialog.h
* @brief the input dialog.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the input dialog Class,
* which provides a simple convenience dialog
* to get a single value from the user. The
* file also contains the OutputNameInputDialog
* Class, which is to get a output file's name.
*/

#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QDialogButtonBox;
class QValidator;
class QVBoxLayout;

class InputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InputDialog(QWidget *parent = 0, Qt::WindowFlags=0);

    void setTitle(const QString &title);
    void setLabelText(const QString &label);
    void setText(const QString &text);
    void setValidator(const QValidator *validator);
    void setInputMask(const QString &inputMask);
    void setPlaceholderText(const QString &placeholderText);

    QString getLabelText();
    QString getText();

    static QString getText(QWidget *parent, const QString &title, const QString &label, const QString &text = QString(), const QValidator *validator = 0,const QString &inputMark = QString(), bool*ok=0, Qt::WindowFlags flags=0);
private slots:
    void checkValid(const QString &text);
protected:
    QLabel *label;
    QLineEdit *text;
    QDialogButtonBox *buttonBox;
    QVBoxLayout *layout;
};

class OutputNameInputDialog : public InputDialog
{
    Q_OBJECT
public:
    explicit OutputNameInputDialog(QWidget *parent = 0, Qt::WindowFlags flags=0);
private:
    QLabel *mesLabel;
    QStringList message;
};

#endif // INPUTDIALOG_H
