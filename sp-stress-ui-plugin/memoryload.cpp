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
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QFuture>
#include <QtConcurrentRun>

static size_t pagesize;

/* Filling the memory area is done in separate thread so that the UI will not
 * block. This can take several seconds for large memory areas.
 */
static void writeMemory(void *ptr, size_t bytes,
		bool randomizedFill, char fillPattern)
{
	qDebug() << Q_FUNC_INFO << ":"
		<< ptr << "-"
		<< (void *)((char *)ptr + bytes)
		<< "randomized:" << randomizedFill
		<< "pattern:" << int(fillPattern);
	if (randomizedFill) {
		long *p = (long *)ptr;
		unsigned cnt = bytes / sizeof(*p);
		while (cnt-- > 0)
			*p++ = random();
	} else {
		memset(ptr, fillPattern, bytes);
	}
}

/* Reading the memory area is done in separate thread so that the UI will not
 * block. This can also take several seconds, although it tends to be much
 * faster than the writing.
 */
static void readMemory(void *ptr, size_t bytes)
{
	qDebug() << Q_FUNC_INFO << ":" << ptr << "-"
		<< (void *)((char *)ptr + bytes);
	volatile char dummy;
	char *start, *end;
	start = (char *)ptr;
	end = ((char *)ptr) + bytes;
	while (start < end) {
		dummy = *start;
		start += 4096;
	}
}

MemoryLoad::MemoryLoad(QObject *parent)
	: QObject(parent)
	, _fillPattern(0)
	, _randomizedFill(true)
	, _allocationFailed(false)
	, _busy(false)
	, _allocationSize(0)
	, _buffer(NULL)
	, _smapsFP(NULL)
{
	long _sc_pagesize = sysconf(_SC_PAGESIZE);
	if (_sc_pagesize == -1)
		pagesize = 4096;
	else
		pagesize = _sc_pagesize;
	connect(&_bgWorker, SIGNAL(finished()), SLOT(bgOperationFinished()));
	_smapsFP = fopen("/proc/self/smaps", "r");
}

void MemoryLoad::freeAllocation()
{
	if (_busy)
		return;
	if (!_buffer)
		return;
	workStarts();
	munmap((char *)_buffer - pagesize, _allocationSize + 2*pagesize);
	_buffer = NULL;
	_allocationSize = 0;
	workEnds();
	setAllocationFailed(false);
	emit allocatedMemoryChanged();
	emit allocationInfoChanged();
}

void MemoryLoad::bgOperationFinished()
{
	qDebug() << Q_FUNC_INFO;
	QFuture<void> future = _bgWorker.future();
	// Should be finished at this point already, but I guess this will not
	// hurt.
	future.waitForFinished();
	workEnds();
	emit allocationInfoChanged();
}

/* We show the /proc/pid/smaps fragment for the allocated buffer in the UI, so
 * create a separate mapping for it.
 *
 * Note that the kernel can collapses adjacent anonymous mappings, and I would
 * really really like to have our buffer in it's own virtual memory area.
 * Hence trick the system by allocating two extra pages, and change the
 * protection to PROT_NONE for those. That way the kernel should not be able to
 * collapse anything.
 *
 *    -----------------------------------------------------------
 *    | <1 page>  | <... _allocationSize bytes> ...> | <1 page> |
 *    -----------------------------------------------------------
 *          ^                     ^                        ^
 *          |                     |                        |
 *          \           PROT_READ | PROT_WRITE             /
 *           \                                            /
 *            \                                          /
 *             \________________________________________/
 *                                |
 *                            PROT_NONE
 */
void MemoryLoad::setAllocatedMemory(int megaBytes)
{
	if (_busy)
		return;
	if (megaBytes < 0)
		return;
	size_t bytes = megaBytes * 1024 * 1024;
	if (bytes == _allocationSize)
		return;
	workStarts();
	void *newptr;
	if (_buffer == NULL) {
		qDebug() << Q_FUNC_INFO << ": creating new memory map.";
		newptr = mmap(NULL, bytes + 2*pagesize,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	} else {
		mprotect((char *)_buffer - pagesize, pagesize,
				PROT_READ | PROT_WRITE);
		mprotect((char *)_buffer + _allocationSize, pagesize,
				PROT_READ | PROT_WRITE);
		newptr = mremap((char *)_buffer - pagesize,
				_allocationSize + 2*pagesize,
				bytes + 2*pagesize,
				MREMAP_MAYMOVE);
	}
	if (newptr == MAP_FAILED) {
		workEnds();
		setAllocationFailed(true);
	} else {
		qDebug() << Q_FUNC_INFO << ": memory map now at" << newptr <<
			"-" << (void *)((char *)newptr + bytes);
		mprotect(newptr, pagesize, PROT_NONE);
		mprotect((char *)newptr + pagesize + bytes, pagesize, PROT_NONE);
		setAllocationFailed(false);
		newptr = (char *)newptr + pagesize;
		_buffer = newptr;
		_allocationSize = bytes;
		emit allocatedMemoryChanged();
		emit allocationInfoChanged();
		startWrite();
	}
	qDebug() << Q_FUNC_INFO << "OK!";
}

void MemoryLoad::setRandomizedFill(bool newState)
{
	if (newState != _randomizedFill) {
		_randomizedFill = newState;
		emit randomizedFillChanged();
	}
}

void MemoryLoad::setAllocationFailed(bool newState)
{
	if (newState != _allocationFailed) {
		_allocationFailed = newState;
		emit allocationFailedChanged();
	}
}

void MemoryLoad::updateAllocationInfo()
{
	emit allocationInfoChanged();
}

static QString get_smaps_entries(FILE *);
static int fseek_smaps_blob(FILE *);

#ifdef MAEMO6

/* Optimization hack for faster parsing of /proc/self/smaps on Harmattan. After
 * reading the first line that tells the mapping, we know that the following 13
 * lines are 28 bytes each.
 */
#define HARMATTAN_SMAPS_BLOB_BYTES (13*28)

/*
3ac2e000-3ac31000 rw-p 00000000 00:00 0 
Size:                 12 kB
Rss:                   8 kB
Pss:                   8 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         8 kB
Referenced:            8 kB
Anonymous:             8 kB
Swap:                  0 kB
KernelPageSize:        4 kB
MMUPageSize:           4 kB
Locked:                0 kB
*/

static QString
get_smaps_entries(FILE *fp)
{
	char buf[HARMATTAN_SMAPS_BLOB_BYTES];
	size_t r = fread(buf, 1, sizeof(buf), fp);
	if (r != sizeof(buf))
		return QString();
	buf[sizeof(buf)-1] = '\0';
	return QString(buf);
}

static int
fseek_smaps_blob(FILE *fp)
{
	return fseek(fp, HARMATTAN_SMAPS_BLOB_BYTES, SEEK_CUR);
}

#else

static QString
get_smaps_entries(FILE *fp)
{
	char *line = NULL;
	size_t line_n = 0;
	QString ret;
	while (getline(&line, &line_n, fp) != -1) {
		if (line[0] < 'A'
		    || line[0] > 'Z'
		    || strchr(line, ':') == NULL)
			break;
		ret += line;
	}
	free(line);
	return ret;
}

static int
fseek_smaps_blob(FILE *)
{
	return 0;
}

#endif

QString MemoryLoad::allocationInfo() const
{
	qDebug() << Q_FUNC_INFO;
	static char *line = NULL;
	static size_t line_n = 0;
	QString smaps;
	ssize_t ret;
	if (_buffer == NULL) {
		qDebug() << Q_FUNC_INFO << ": _buffer not yet allocated.";
		goto out;
	}
	if (!_smapsFP) {
		qDebug() << Q_FUNC_INFO
			<< ": unable to open /proc/self/smaps!";
		goto out;
	}
	rewind(_smapsFP);
	while (true) {
		ret = getline(&line, &line_n, _smapsFP);
		if (ret == -1) {
			qDebug() << Q_FUNC_INFO
				<< ": getline() returns -1, breaking out.";
			goto out;
		}
		unsigned start_addr = 0, end_addr = 0;
		if (sscanf(line, "%x-%x", &start_addr, &end_addr) != 2) {
#ifdef MAEMO6
			qDebug() << Q_FUNC_INFO 
				<< ": unable to parse start & end addresses"
				   ", line:" << line;
			goto out;
#else
			continue;
#endif
		}
		void *start_ptr = (void *)start_addr;
		void *end_ptr = (void *)end_addr;
		if (start_ptr <= _buffer && _buffer < end_ptr) {
			/* OK, found it! */
			smaps += line;
			smaps += get_smaps_entries(_smapsFP);
			goto out;
		} else {
			if (fseek_smaps_blob(_smapsFP) == -1) {
				qDebug() << Q_FUNC_INFO << ": fseek() failed.";
				goto out;
			}
		}
	}
out:
	return smaps;
}

void MemoryLoad::memoryRead()
{
	if (_busy)
		return;
	QFuture<void> future = QtConcurrent::run(readMemory,
			_buffer, _allocationSize);
	_bgWorker.setFuture(future);
	workStarts();
}

void MemoryLoad::memoryWrite()
{
	if (_busy)
		return;
	startWrite();
}

void MemoryLoad::startWrite()
{
	QFuture<void> future = QtConcurrent::run(writeMemory,
			_buffer, _allocationSize,
			_randomizedFill, _fillPattern);
	_bgWorker.setFuture(future);
	workStarts();
}

void MemoryLoad::workStarts()
{
	if (_busy)
		return;
	_busy = true;
	emit busyChanged();
}

void MemoryLoad::workEnds()
{
	if (!_busy)
		return;
	_busy = false;
	emit busyChanged();
}
