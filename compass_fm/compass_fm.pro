#-------------------------------------------------
#
# Project created by QtCreator 2014-11-14T14:35:34
#
#-------------------------------------------------

QT       += core gui widgets

QT       -= gui

TARGET = compass_fm
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    compass_fm.cpp \
    parseCSV.cpp \
    ../strtools.cpp

HEADERS += \
    parseCSV.h \
    ../strtools.h
