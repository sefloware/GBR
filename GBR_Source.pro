#-------------------------------------------------
#
# Project created by QtCreator 2013-06-20T15:02:16
#
#-------------------------------------------------

QT       += core gui sql network

qtHaveModule(printsupport): QT += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GBR_Source
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    project.cpp \
    tablefilter.cpp \
    tableview.cpp \
    inputdialog.cpp \
    qtsingleapplication.cpp \
    qtlocalpeer.cpp \
    qtlockedfile_win.cpp \
    qtlockedfile_unix.cpp \
    qtlockedfile.cpp \
    cppreader.cpp \
    cppposquery.cpp \
    cppobjlist.cpp \
    editorhighlighter.cpp \
    cppclass.cpp \
    cppvariable.cpp \
    cppfunction.cpp \
    mdeditor.cpp \
    cppenumespace.cpp \
    tablecalculator.cpp \
    mddebug.cpp \
    item.cpp \
    outputdialog.cpp \
    richtextedit.cpp \
    richtextwidget.cpp \
    startpage.cpp \
    config.cpp \
    stcodeeditor.cpp \
    stproject.cpp \
    stprojecteditor.cpp \
    editor1.cpp \
    editor2.cpp \
    editormap.cpp \
    editorobjmodel.cpp \
    mdparout.cpp \
    pjanalyser.cpp \
    pjmodeler.cpp \
    seed.cpp \
    seeddialog.cpp \
    simulationbar.cpp \
    stcode.cpp \
    stscparser.cpp \
    table.cpp \
    qrcfile.cpp

HEADERS  += mainwindow.h \
    project.h \
    tablefilter.h \
    tableview.h \
    inputdialog.h \
    qtsingleapplication.h \
    qtlocalpeer.h \
    qtlockedfile.h \
    cppreader.h \
    editorcompleter.h \
    editorhighlighter.h \
    cppposquery.h \
    cppobjlist.h \
    cppkeywords.h \
    cppclass.h \
    cppvariable.h \
    cppfunction.h \
    mdeditor.h \
    stproject.h \
    startpage.h \
    cppenumespace.h \
    tablecalculator.h \
    mddebug.h \
    item.h \
    mdparoutdelegate.h \
    outputdialog.h \
    richtextedit.h \
    richtextwidget.h \
    config.h \
    stcodeeditor.h \
    stprojecteditor.h \
    editor2.h \
    editor1.h \
    editorobjmodel.h \
    editormap.h \
    pjanalyser.h \
    pjmodeler.h \
    mdparout.h \
    qrcfile.h \
    seeddialog.h \
    simulationbar.h \
    stcode.h \
    seed.h \
    stscparser.h \
    table.h \
    tabledelegate.h

FORMS    += \
    outputdialog.ui

RESOURCES += \
    source.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/stylesheet
INSTALLS += target

OTHER_FILES += \
    richtextedit/textedit.pro

RC_FILE = GBR_WIN.rc
