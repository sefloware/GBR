/** @file main.cpp
* @brief the main Function.
* @author Runhua Li
* @date 2013.7
* @version v1.0
* @note
* The file is about the main Function.
*/

#include "mainwindow.h"
#include "cppkeywords.h"
#include "stscparser.h"
#include "qtsingleapplication.h"
#include <QFile>
#include <QTreeView>
#include <QTableView>
#include <QStandardItemModel>

int main(int argc, char *argv[])
{
    QtSingleApplication app(argc, argv);

    Q_INIT_RESOURCE(source);

    if (app.isRunning())
    {
        app.sendMessage("raies");
        return EXIT_SUCCESS;
    }

    QFont font(qApp->font());
    font.setPointSize(14);
    font.setStyleStrategy(QFont::PreferAntialias);
    qApp->setFont(font);

    QFile file(":/file/files/coffee.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QString::fromLatin1(file.readAll());
    qApp->setStyleSheet(styleSheet);

    MainWindow w;
    app.setActivationWindow(&w);
    w.show();

    return app.exec();
}

