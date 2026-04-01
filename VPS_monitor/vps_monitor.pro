QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 release

TARGET = vps_monitor
TEMPLATE = app

# ─────────────────────────────
# Sources
# ─────────────────────────────
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    servermonitor.cpp \
    metriccardwidget.cpp \
    eventlogwidget.cpp \
    statusbadge.cpp \
    sparklinewidget.cpp

# ─────────────────────────────
# Headers
# ─────────────────────────────
HEADERS += \
    mainwindow.h \
    servermonitor.h \
    metriccardwidget.h \
    eventlogwidget.h \
    statusbadge.h \
    sparklinewidget.h

# ─────────────────────────────
# Resources
# ─────────────────────────────
RESOURCES += \
    resources.qrc

# ─────────────────────────────
# Warnings (pro)
# ─────────────────────────────
QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic

# ─────────────────────────────
# Optimización release
# ─────────────────────────────
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += -O2
}

# ─────────────────────────────
# Deployment (Linux opcional)
# ─────────────────────────────
unix:!android {
    target.path = /opt/$${TARGET}/bin
    INSTALLS += target
}