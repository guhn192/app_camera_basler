QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    basler_camera.cpp

HEADERS += \
    mainwindow.h \
    basler_camera.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# OpenCV library
INCLUDEPATH += /usr/include/opencv4/
LIBS += -L/usr/lib/x86_64-linux-gnu/
LIBS += -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgcodecs \
        -lopencv_imgproc

# Basler Pylon
INCLUDEPATH += /opt/pylon/include
LIBS += -L/opt/pylon/lib/
LIBS += -lpylonbase \
        -lpylonutility \
        -lpylonc \
        -lGCBase_gcc_v3_1_Basler_pylon_v3 \
        -lGenApi_gcc_v3_1_Basler_pylon_v3
QMAKE_LFLAGS += -Wl,-rpath,/opt/pylon/lib/ 