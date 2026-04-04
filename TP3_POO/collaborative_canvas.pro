QT += core gui widgets network

CONFIG += c++17

TARGET = collaborative_canvas
TEMPLATE = app

SOURCES += \
    main.cpp \
    DrawingModel.cpp \
    CanvasView.cpp \
    SyncManager.cpp \
    MainWindow.cpp

HEADERS += \
    DrawingModel.h \
    CanvasView.h \
    SyncManager.h \
    MainWindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
