/*
 * Copyright (C) 2011 Nokia Corporation.
 *
 * Author: Pertti Kellomäki <pertti.kellomaki@nokia.com>
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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "memoryload.h"
#include "cgroupinfo.h"

#include <QtDeclarative>
#include <QDeclarativeExtensionPlugin>

class SpMemloadPlugin : public QDeclarativeExtensionPlugin
{
	Q_OBJECT
public:
	void registerTypes(const char *uri);
};

#endif /* PLUGIN_H */
