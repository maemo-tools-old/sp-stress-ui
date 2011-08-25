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

#include "cgroupinfo.h"
#include <QDebug>
#include <QString>
#include <QStringList>

QString CgroupInfo::cgroup() const
{
	qDebug() << Q_FUNC_INFO;
	static char *line = NULL;
	static size_t line_n = 0;
	QString cgroup;
	QStringList splitted;
	ssize_t ret;
	FILE *fp;
	fp = fopen("/proc/self/cgroup", "r");
	if (!fp)
		goto out;
	ret = getline(&line, &line_n, fp);
	if (ret == -1)
		goto out;
	cgroup = line;
	splitted = cgroup.split(':');
	if (splitted.count() != 3)
		goto out;
	cgroup = splitted[2];
	cgroup.chop(1);
out:
	if (fp)
		fclose(fp);
	return cgroup;
}

int CgroupInfo::memoryUsage() const
{
	int usage = -1;
	FILE *fp;
	QString cg = cgroup();
	QString memlimit = QLatin1String("/syspart") + cg
		+ QLatin1String("/memory.usage_in_bytes");
	fp = fopen(memlimit.toAscii(), "r");
	if (!fp)
		goto out;
	if (fscanf(fp, "%d", &usage) != 1)
		goto out;
out:
	if (fp)
		fclose(fp);
	qDebug() << Q_FUNC_INFO << ":" << usage;
	return usage;
}

int CgroupInfo::swapUsage() const
{
	qDebug() << Q_FUNC_INFO;
	int swapUsage = -1;
	int memswUsage = -1;
	FILE *fp;
	QString cg = cgroup();
	QString memlimit = QLatin1String("/syspart") + cg
		+ QLatin1String("/memory.memsw.usage_in_bytes");
	fp = fopen(memlimit.toAscii(), "r");
	if (!fp) {
		qDebug() << Q_FUNC_INFO << ": unable to open" << memlimit;
		goto out;
	}
	if (fscanf(fp, "%d", &memswUsage) != 1) {
		qDebug() << Q_FUNC_INFO << ": unable to read integer.";
		goto out;
	}
	swapUsage = memswUsage - memoryUsage();
	swapUsage = qMax(0, swapUsage);
out:
	if (fp)
		fclose(fp);
	qDebug() << Q_FUNC_INFO << ":" << swapUsage;
	return swapUsage;
}

int CgroupInfo::memoryLimit() const
{
	qDebug() << Q_FUNC_INFO;
	int limit = -1;
	FILE *fp;
	QString cg = cgroup();
	QString memlimit = QLatin1String("/syspart") + cg
		+ QLatin1String("/memory.limit_in_bytes");
	fp = fopen(memlimit.toAscii(), "r");
	if (!fp) {
		qDebug() << Q_FUNC_INFO << ": unable to open" << memlimit;
		goto out;
	}
	if (fscanf(fp, "%d", &limit) != 1) {
		qDebug() << Q_FUNC_INFO << ": unable to read integer.";
		goto out;
	}
out:
	if (fp)
		fclose(fp);
	qDebug() << Q_FUNC_INFO << ":" << limit;
	return limit;
}

void CgroupInfo::update()
{
	emit cgroupChanged();
	emit memoryUsageChanged();
	emit swapUsageChanged();
	emit memoryLimitChanged();
}
