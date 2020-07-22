#-------------------------------------------------
#
# Project created by QtCreator 2014-09-25T13:55:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Duplicates
TEMPLATE = app

win32:RC_ICONS += D_icon.ico

SOURCES += main.cpp\
        duplicates.cpp \
    comparethread.cpp

HEADERS  += duplicates.h \
    comparethread.h

FORMS    += duplicates.ui
