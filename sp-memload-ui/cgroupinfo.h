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

#ifndef CGROUPINFO
#define CGROUPINFO

#include <QObject>

class CgroupInfo : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString cgroup
			READ cgroup
			NOTIFY cgroupChanged)
	Q_PROPERTY(int memoryUsage
			READ memoryUsage
			NOTIFY memoryUsageChanged)
	Q_PROPERTY(int swapUsage
			READ swapUsage
			NOTIFY swapUsageChanged)
	Q_PROPERTY(int memoryLimit
			READ memoryLimit
			NOTIFY memoryLimitChanged)

public:
	CgroupInfo(QObject *parent = 0) : QObject(parent) {}
	~CgroupInfo() {}
	Q_INVOKABLE QString cgroup() const;
	Q_INVOKABLE void update();
	int memoryUsage() const;
	int swapUsage() const;
	int memoryLimit() const;

signals:
	void cgroupChanged();
	void memoryUsageChanged();
	void swapUsageChanged();
	void memoryLimitChanged();
};

#endif /* CGROUPINFO */
