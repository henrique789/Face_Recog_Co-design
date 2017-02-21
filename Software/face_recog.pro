TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
#CONFIG += -std=c++11
#CONFIG += c++11
#CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS += -std=c++0x


SOURCES += main.cpp \
    apc.cpp \
    autofaces.cpp

TARGET = noqt

LIBS += `pkg-config opencv --cflags --libs`

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann
LIBS += -lopencv_nonfree

HEADERS += \
    apc.h \
    autofaces.h
