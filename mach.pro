#-------------------------------------------------
#
# Project created by QtCreator 2013-09-03T22:27:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mach
TEMPLATE = app


SOURCES += main.cpp \
    ui_qt_main.cpp \
    ui_qt_dialog.cpp

HEADERS  += \
    ui_qt_main.h \
    ui_qt_dialog.h

FORMS    += \
    ui_qt_main.ui \
    ui_qt_dialog.ui

QMAKE_CXXFLAGS += -std=c++11

OTHER_FILES += \
    stdlib.mac
