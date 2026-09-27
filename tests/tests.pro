QT += core sql testlib
CONFIG += console testcase c++17
TEMPLATE = app
TARGET = environment_monitor_tests

INCLUDEPATH += $$PWD/..
SOURCES += $$PWD/test_thresholds.cpp $$PWD/../thresholdstore.cpp
HEADERS += $$PWD/../thresholdstore.h $$PWD/../monitorlogic.h
