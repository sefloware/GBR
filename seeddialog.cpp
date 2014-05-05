/** @file seeddialog.cpp
* @brief getting seed from the WWW.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the progress dialog of getting
* seed from the WWW.
*/

#include "seeddialog.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLabel>
#include <QDebug>

SeedProgressDialog::SeedProgressDialog(int count,QWidget *parent)
    : QProgressDialog(parent)
{
    this->setAutoClose(true);
    this->setLabel(new QLabel("Getting seeds from internate..."));

    const QString source = "http://www.random.org/integers/?num=%1&min=1&max=1000000000&col=1&base=10&format=plain&rnd=new";
    nam = new QNetworkAccessManager(this);
    QNetworkRequest rq(QUrl(source.arg(QString::number(count))));
    reply = nam->get(rq);

    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this,SLOT(set_maximum_value(qint64,qint64)) );
    connect(reply, SIGNAL(finished()),this,SLOT(replyFinished()) );
}

SeedProgressDialog::~SeedProgressDialog()
{
    delete reply;
}

void SeedProgressDialog::replyFinished()
{
    data = reply->readAll();
    this->accept();
}

void SeedProgressDialog::set_maximum_value(qint64 received, qint64 total)
{
    this->setMaximum(total);
    this->setValue(received);
}
