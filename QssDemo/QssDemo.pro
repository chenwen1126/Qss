#-------------------------------------------------
#
# Project created by QtCreator 2018-02-06T10:25:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QssDemo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../Qss/Qss.cpp

HEADERS  += mainwindow.h \
    ../Qss/Qss.h

FORMS    += mainwindow.ui

RESOURCES += \
    ../Qss/qss.qrc \
    app.qrc
