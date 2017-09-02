#-------------------------------------------------
#
# Project created by QtCreator 2015-12-02T05:58:04
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = Estabilizador
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -LC:\OpenCV2.2\lib \
    -lopencv_ffmpeg220 \
    -lopencv_core220 \
    -lopencv_highgui220 \
    -lopencv_video220 \
    -lopencv_imgproc220 \
    -lopencv_features2d220 \
    -lopencv_objdetect220

INCLUDEPATH += C:\OpenCV2.2\include\

SOURCES += main.cpp
