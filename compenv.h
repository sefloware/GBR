/** @file compenv.h
* @brief the computing environment.
* @author Runhua Li
* @date 2013.5
* @version v1.0
* @note
* The file is about the computing envrionment.
*/

#ifndef COMPENV_H
#define COMPENV_H

#include <QDialog>
#include "ui_compenv.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
QT_END_NAMESPACE

namespace CE
{
enum ItemRole
{
    NameRole = Qt::EditRole,
    ThreadNumRole = Qt::UserRole+1,
    CNameRole,
    CIncludepathRole ,
    COptimizationRole,
    CMacroRole,
    COutputRole,
    PlatformRole
};

void generateBatch(const QString &path, const QModelIndex &index, QWidget *parent = 0);
QString debugCompilerCommand(const QModelIndex &index);
QString compilerCommand(const QString &seeds, const QModelIndex &index) ;
bool copyRunshTo(const QString &source,const QString &path,const QModelIndex &index);
bool copyPBSshTo(const QString &source,const QString &path,const QModelIndex &index);
bool copyBatTo(const QString &source, const QString &path,const QModelIndex &index) ;

enum PlatForm{WindowsCmd=1,Posix,PosixPBS};
std::map<std::string,PlatForm> &paltformmap();
}

class CEDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CEDialog(QWidget *parent  = 0) :
        QDialog(parent,Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
        ui(new Ui::CEDialog)
    { ui->setupUi(this); }

    ~CEDialog()
    { delete ui; }

    void setPlainText(const QString &text)
    { ui->plainTextEdit->setPlainText(text); }

    QString plainText() const
    { return ui->plainTextEdit->toPlainText(); }

private:
    Ui::CEDialog *ui;
};

#endif // COMPENV_H
