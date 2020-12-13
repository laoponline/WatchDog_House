#-------------------------------------------------
#
# Project created by QtCreator 2020-11-26T00:27:30
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT += widgets printsupport
QT       += serialport



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Dog_House
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        dialog_dog_config.cpp \
        dialog_system_config.cpp \
        dog_widget.cpp \
        main.cpp \
        mainwindow.cpp \
        qcustomplot.cpp \
        system_resource_usage.cpp

HEADERS += \
        dialog_dog_config.h \
        dialog_system_config.h \
        dog_widget.h \
        globle_define.h \
        mainwindow.h \
        qcustomplot.h \
        system_resource_usage.h

FORMS += \
        dialog_dog_config.ui \
        dialog_system_config.ui \
        dog_widget.ui \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc
