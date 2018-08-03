TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    mod_class.cpp

LIBS += -lsdl2

HEADERS += \
    mod_class.h
