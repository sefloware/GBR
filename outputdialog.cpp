/** @file outputdialog.cpp
* @brief the output dialog.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the output dialog Class,
* which provides a simple convenience dialog
* to get a output path from the user.
*/

#include "outputdialog.h"
#include "config.h"
#include "ui_outputdialog.h"
#include <QFileDialog>
#include <QMessageBox>

OutputDialog::OutputDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    ui(new Ui::OutputDialog)
{
    ui->setupUi(this);

    connect(ui->lineEdit,SIGNAL(textChanged(QString)), this,SLOT(textChanged()) );
    connect(ui->lineEdit2,SIGNAL(textChanged(QString)), this,SLOT(textChanged()) );
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(exploreDirectory()) );
    connect(this,SIGNAL(accepted()),this,SLOT(savePath()) );

    ui->checkBox->setChecked(false);
    ui->lineEdit2->setText(defaultPath());
}

OutputDialog::~OutputDialog()
{
    delete ui;
}


QString OutputDialog::defaultPath()
{
    if(myInfos().size()<2)
        return QDir::homePath();
    return  myInfos().at(1);
}


void OutputDialog::savePath()
{
    QString defautpath = ui->lineEdit2->text().simplified();

    if(! ui->checkBox->isChecked() || defautpath.isEmpty())
        return;

    QString confpath = absolutePath(ConfigFolder);
    if( confpath.isEmpty())
        return; //For simplification

    QFile file( QDir(confpath).absoluteFilePath("default_path") );

    if(! file.remove() || ! file.open(QIODevice::WriteOnly))
        return; //For simplification

    file.write(defautpath.toLocal8Bit());
}

void OutputDialog::textChanged()
{
    const QString path = ui->lineEdit2->text().simplified();
    const QString name = ui->lineEdit->text().simplified();

    if(path.isEmpty() || name.isEmpty())
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    const QDir dir(path);
    if( !dir.exists() )
    {
        ui->label->setText(tr("<font color=red>The path doesn't exist and it will be made</font>") );
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else if( dir.exists(name) )
    {
        ui->label->setText("<font color=red><b>" + name + "</b> already exist in the path:\n<b>" + path + "</b></font>" );
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        ui->label->clear();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void OutputDialog::exploreDirectory()
{
    QString path = QFileDialog::getExistingDirectory (this, tr("Directory Dialog"), ui->lineEdit2->text());
    if(! path.isEmpty())
        ui->lineEdit2->setText(path);
}

QString OutputDialog::getPath()
{
    const QString name = ui->lineEdit->text().simplified();
    QString path = ui->lineEdit2->text().simplified();
    path.append(QDir::separator() ).append(name);

    if(! QDir().mkpath(path) )
    {
        QMessageBox::critical(this, tr("Error"), "Failed to make the path:\n" + path, QMessageBox::Ok, QMessageBox::Ok);
        return QString();
    } else {
        return path;
    }
}

void OutputDialog::setNameValidator(const QValidator *nameValidator)
{ ui->lineEdit->setValidator( nameValidator); }
