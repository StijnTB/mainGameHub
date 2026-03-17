QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += sdl2
}

CONFIG += c++17
# Include path for headers
INCLUDEPATH += "C:\SDL2\SDL2-2.32.10\x86_64-w64-mingw32/include"

# Library path and specific libraries
LIBS += -L"C:\SDL2\SDL2-2.32.10\x86_64-w64-mingw32/lib" \
        -lSDL2main \
        -lSDL2

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    mainGameHub_nl_NL.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../../../5e jaar/informatica/GODOT/asset library/pixilart-drawing (2).png \
    ../../../../5e jaar/informatica/GODOT/asset library/tiny-alchemist-pixilart (1).png
