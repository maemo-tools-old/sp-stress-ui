TEMPLATE = lib
CONFIG += qt plugin
QT += declarative

HEADERS = cgroupinfo.h memoryload.h plugin.h
SOURCES = cgroupinfo.cpp memoryload.cpp plugin.cpp

target.path = /usr/lib/qt4/imports/com/nokia/SpStressUi
other.path = /usr/lib/qt4/imports/com/nokia/SpStressUi
other.files = qmldir

INSTALLS += target other

DEFINES += QT_NO_DEBUG_OUTPUT

!$$MAEMO6 {
	message("Building with Maemo 6 features.")
	DEFINES += MAEMO6
}
