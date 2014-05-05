/** @file inputdialog.cpp
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

#include "inputdialog.h"
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QRegExpValidator>

InputDialog::InputDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent)
{
    if(flags!=0) setWindowFlags(flags);

    layout=new QVBoxLayout(this);

    label=new QLabel(this);

    buttonBox=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    text=new QLineEdit(this);
    connect(text, SIGNAL(textChanged(QString)), this, SLOT(checkValid(QString)));

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(label);
    hl->addWidget(text);
    label->setBuddy(text);

    layout->addLayout(hl);
    layout->addWidget(buttonBox);
}

void InputDialog::setTitle(const QString &title){ setWindowTitle(title); }
void InputDialog::setLabelText(const QString &label){ this->label->setText(label); }
void InputDialog::setText(const QString &text){ this->text->setText(text); }

void InputDialog::setValidator(const QValidator *validator){
    text->setValidator(validator);
    checkValid(text->text());
}

void InputDialog::setInputMask(const QString &inputMask){
    text->setInputMask(inputMask);
    checkValid(text->text());
}

void InputDialog::setPlaceholderText(const QString &placeholderText)
{
    text->setPlaceholderText(placeholderText);
}

QString InputDialog::getLabelText(){ return label->text(); }
QString InputDialog::getText(){ return text->text(); }

void InputDialog::checkValid(const QString &text){

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(this->text->hasAcceptableInput() && !text.isEmpty());
}

QString InputDialog::getText(QWidget *parent, const QString &title, const QString &label, const QString &text, const QValidator *validator, const QString &inputMark, bool *ok, Qt::WindowFlags flags){
    InputDialog r(parent, flags);
    r.setTitle(title);
    r.setLabelText(label);
    r.setText(text);
    if(validator)
        r.setValidator(validator);
    r.setInputMask(inputMark);

    bool bl = (r.exec()==QDialog::Accepted);
    if( ok ) *ok = bl;
    if( bl ) return r.getText();
    else return QString();
}

OutputNameInputDialog::OutputNameInputDialog(QWidget *parent, Qt::WindowFlags flags)
        : InputDialog(parent,flags)
{
    QRegExp regExp("[a-zA-Z_](?:\\d+|\\w*[a-zA-Z]\\d+|\\w*[a-zA-Z_])?\\.[a-zA-Z_]\\w*");
    QRegExpValidator *v = new QRegExpValidator(regExp, this);
    this->setTitle("New Output File");
    this->setLabelText("File Name: ");
    this->setValidator(v);

    mesLabel = new QLabel;
    mesLabel->setText(tr("The suffix of the file name include:<br>"
                         "<b>.st</b>(file for statistic),<br>"
                         "<b>.bin</b>(file for trajectory),<br>"
                         "and other suffix(file for dynamic).") );
    layout->insertWidget(1,mesLabel);
}
