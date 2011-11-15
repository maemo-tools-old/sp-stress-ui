TEMPLATE = app
DEPENDPATH += .
HEADERS += memoryload.h cgroupinfo.h
SOURCES += memoryload.cpp cgroupinfo.cpp main.cpp
RESOURCES += qmlfiles.qrc
QT += declarative

target.path = /usr/bin
desktop.path = /usr/share/applications
desktop.files = sp-memload-ui.desktop
icon.path = /usr/share/icons
icon.files = sp-memload-ui.png

TRANSLATIONS += \
	sp-memload-ui_fi.ts

qm.path = /usr/share/l10n/meegotouch
qm.files = sp-memload-ui_*.qm

INSTALLS += target desktop icon qm
DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG += qdeclarative-boostable

DEFINES += USE_BOOSTER
