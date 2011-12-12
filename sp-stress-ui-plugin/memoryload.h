#ifndef MEMORY_LOAD
#define MEMORY_LOAD

#include <stdio.h>
#include <QObject>
#include <QString>
#include <QFutureWatcher>

class MemoryLoad : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int allocatedMemory
			READ allocatedMemory
			WRITE setAllocatedMemory
			NOTIFY allocatedMemoryChanged)
	Q_PROPERTY(char fillPattern
			READ fillPattern
			WRITE setFillPattern)
	Q_PROPERTY(bool randomizedFill
			READ randomizedFill
			WRITE setRandomizedFill
			NOTIFY randomizedFillChanged)
	Q_PROPERTY(bool allocationFailed
			READ allocationFailed
			NOTIFY allocationFailedChanged)
	Q_PROPERTY(QString allocationInfo
			READ allocationInfo
			NOTIFY allocationInfoChanged)
	Q_PROPERTY(bool busy
			READ busy
			NOTIFY busyChanged)

public:
	MemoryLoad(QObject *parent = 0);
	~MemoryLoad() {}
	int allocatedMemory() const { return _allocationSize / 1024 / 1024; }
	Q_INVOKABLE void setAllocatedMemory(int megaBytes);
	char fillPattern() const { return _fillPattern; }
	Q_INVOKABLE void setFillPattern(char newFillPattern) { _fillPattern = newFillPattern; }
	bool randomizedFill() { return _randomizedFill; }
	Q_INVOKABLE void setRandomizedFill(bool);
	bool allocationFailed() const { return _allocationFailed; }
	QString allocationInfo() const;
	Q_INVOKABLE void updateAllocationInfo();
	Q_INVOKABLE void memoryRead();
	Q_INVOKABLE void memoryWrite();
	bool busy() const { return _busy; }
	Q_INVOKABLE void freeAllocation();

signals:
	void allocatedMemoryChanged();
	void randomizedFillChanged();
	void allocationFailedChanged();
	void allocationInfoChanged();
	void busyChanged();

private:
	void startWrite();
	void setAllocationFailed(bool);
	void workStarts();
	void workEnds();

private slots:
	void bgOperationFinished();

private:
	char _fillPattern;
	bool _randomizedFill : 1;
	bool _allocationFailed : 1;
	bool _busy : 1;
	size_t _allocationSize;
	void *_buffer;
	FILE *_smapsFP;
	QFutureWatcher<void> _bgWorker;
};

#endif /* MEMORY_LOAD */
