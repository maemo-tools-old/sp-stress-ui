/*
 * Copyright (C) 2011 Nokia Corporation.
 *
 * Author: Tommi Rantala <ext-tommi.1.rantala@nokia.com>
 * Contact: Eero Tamminen <eero.tamminen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "memoryload.h"
#include "cgroupinfo.h"
#include <MDeclarativeCache>
#include <QApplication>
#include <QtDeclarative>
#include <QtGlobal>
#include <QTranslator>

Q_DECL_EXPORT int main(int argc, char **argv)
{
	QApplication *app = MDeclarativeCache::qApplication(argc, argv);
	QDeclarativeView *window = MDeclarativeCache::qDeclarativeView();
	QTranslator translator;
	translator.load("sp-memload-ui_" + QLocale::system().name(),
			"/usr/share/l10n/meegotouch");
	app->installTranslator(&translator);
	app->setProperty("NoMStyle", true);
	qmlRegisterType<MemoryLoad>("com.nokia.endurance", 1, 0, "MemoryLoad");
	qmlRegisterType<CgroupInfo>("com.nokia.endurance", 1, 0, "CgroupInfo");
	window->setSource(QUrl("qrc:/main.qml"));
	window->showFullScreen();
	return app->exec();
}
