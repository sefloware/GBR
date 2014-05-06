#include "texteditdialog.h"
#include "ui_texteditdialog.h"
#include <QPushButton>

TextEditDialog::TextEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}


TextEditDialog::~TextEditDialog()
{
    delete ui;
}

void TextEditDialog::setPlainText(const QString &t)
{
    ui->textEdit->setPlainText(t);
}

QString TextEditDialog::plainText()
{
    return ui->textEdit->toPlainText();
}

void TextEditDialog::on_textEdit_textChanged()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(
                ui->textEdit->toPlainText().simplified().isEmpty());
}
