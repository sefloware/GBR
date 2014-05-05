/** @file mdeditor.h
* @brief the modeling editor.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the modeling editor Class.
*/

#ifndef MDEDITOR_H
#define MDEDITOR_H

#include "editor2.h"
#include "assert.h"
#include <QDir>
#include <QMultiMap>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QToolBar;
QT_END_NAMESPACE

class MultiMap : public QMultiMap<QString,QString>
{
public:
    MultiMap &operator =(const QStringList &other)
    {
        this->clear();
        foreach (const QString &it, other)
        {
            assert(!it.at(it.length()-1).isSpace() );
            this->insert(it.right(1),it);
        }
        return *this;
    }
};

class MainEditor : public Editor2
{
    Q_OBJECT
public:
    explicit MainEditor(QWidget *parent = 0);
    void setConstStrings(const QStringList &conststrings)
    { conststrs = conststrings; }
    QStandardItemModel *localObjModel() const
    { return objModel; }

    void updateEnv();

    QTextCursor selectRegion(const QRegExp &begin,const QRegExp &end, const QTextCursor &from = QTextCursor());
    QTextCursor selectRegion(const QString &begin, const QString &end, const QTextCursor &from = QTextCursor());
    QTextCursor selectStruct(const QString &header, const QTextCursor &from, bool includeBorder = true);

    QList<QTextCursor> specialRegionsOf(const QString &name);

    void setReadOnly(bool ro);
    void setReadOnly(const QString &str);
public slots:
    void updateObjModel();
protected:
    void keyPressEvent(QKeyEvent *e);
    bool canInsertFromMimeData(const QMimeData *) const;
    void insertFromMimeData(const QMimeData *source);
private slots:
    void updateRWstateUponSelection(); //update read-write state.
    void updateRWstateUponCursor();
private:
    bool canInputCh(const QString &chkey, QString &matchedStr);
private:
    MultiMap conststrs;
    QStandardItemModel *objModel;

    bool readonly;
};

#endif // MDEDITOR_H
