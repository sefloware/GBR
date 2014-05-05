/** @file seeddialog.h
* @brief getting seed from the WWW.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the progress dialog of getting
* seed from the WWW.
*/

#ifndef SEEDDIALOG_H
#define SEEDDIALOG_H

#include <QProgressDialog>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

class SeedProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    SeedProgressDialog(int count, QWidget *parent = 0);
    ~SeedProgressDialog();
private slots:
    void replyFinished();
    void set_maximum_value(qint64 received, qint64 total);
public:
    QByteArray data;
private:
    QNetworkAccessManager *nam;
    QNetworkReply *reply;
};

#endif // SEEDDIALOG_H
