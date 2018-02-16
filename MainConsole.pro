#-------------------------------------------------
#
# Project created by QtCreator 2015-03-20T08:57:15
#
#-------------------------------------------------

QT       += core gui printsupport xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MainConsole
TEMPLATE = app

INCLUDEPATH += /usr/local/qwt-6.1.3/include/
LIBS += -L/usr/local/qwt-6.1.3/lib/ -lqwt

SOURCES += main.cpp\
        mainconsole.cpp

HEADERS  += mainconsole.h \
    xvmc.h \
    mInfo.h \
    user.h

FORMS    += mainconsole.ui

DEFINES  += XVMC

# CONFIG   += qwt
