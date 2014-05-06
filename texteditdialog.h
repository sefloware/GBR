#ifndef TEXTEDITDIALOG_H
#define TEXTEDITDIALOG_H

#include <QDialog>

namespace Ui {
class TextEditDialog;
}

class TextEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextEditDialog(QWidget *parent = 0);
    ~TextEditDialog();

    void setPlainText(const QString &t);
    QString plainText();
private slots:
    void on_textEdit_textChanged();

private:
    Ui::TextEditDialog *ui;
};

#endif // TEXTEDITDIALOG_H
