QT += core gui widgets sql
CONFIG += c++17
TEMPLATE = app
TARGET = 03_test

SOURCES += \
    main.cpp \
    widget.cpp \
    setdialog.cpp \
    historydialog.cpp \
    thresholdstore.cpp

HEADERS += \
    widget.h \
    setdialog.h \
    historydialog.h \
    thresholdstore.h \
    monitorlogic.h

FORMS += \
    widget.ui \
    setdialog.ui \
    historydialog.ui
