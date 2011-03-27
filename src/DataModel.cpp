#include "DataModel.h"

#include <QFileSystemWatcher>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QFile>

DataModel::DataModel(QObject *parent) : QStandardItemModel(parent)
{
	setColumnCount(4);
	setHeaderData(0, Qt::Horizontal, "Date");
	setHeaderData(1, Qt::Horizontal, "Subject");
	setHeaderData(2, Qt::Horizontal, "Maze");
	setHeaderData(3, Qt::Horizontal, "Log File");
	
	watcher = new QFileSystemWatcher(QStringList("logs/"), this);
	connect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(slot_DirectoryChanged(const QString &)));
	connect(watcher, SIGNAL(fileChanged(const QString &)), this, SLOT(slot_FileChanged(const QString &)));
	
	_CrawlDirectory(QDir("logs/"));
}

DataModel::~DataModel()
{
}

void DataModel::slot_DirectoryChanged(const QString &dirPath)
{
	_CrawlDirectory(QDir(dirPath));
}

void DataModel::slot_FileChanged(const QString &filePath)
{
	if (!QFile::exists(filePath))
	{
		const QFileInfo info(filePath);
		const FileMap::iterator it = logfiles.find(info.absoluteFilePath());
		if (it != logfiles.end())
		{
			removeRow(it.value().row());
			logfiles.erase(it);
		}
	}
}
// #include <cstdio>
void DataModel::_CrawlDirectory(const QDir &dir)
{
	if (!dir.exists())
		return;
	watcher->addPath(dir.absolutePath() + "/");
	// printf("Crawling '%s'\n", dir.path().toAscii().data());
	const QFileInfoList entries = dir.entryInfoList(QStringList("*.txt"), QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);
	for (QFileInfoList::const_iterator it = entries.begin(); it != entries.end(); ++it)
	{
		// printf("Considering '%s'\n", it->filePath().toAscii().data());
		if (it->isDir())
			_CrawlDirectory(it->filePath());
		else if (it->isFile())
			_AddLogFile(it->filePath());
	}
}

void DataModel::_AddLogFile(const QString &filename)
{
	const QFileInfo info(filename);
	if (logfiles.contains(info.absoluteFilePath()))
		return;
	
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	
	const QString mazeHeader    = "# Maze: ";
	const QString subjectHeader = "# Subject: ";
	const QString dateHeader    = "# Date: ";
	
	QString subjectName;
	QString mazeName;
	QDateTime timestamp;
	
	while (!file.atEnd())
	{
		const QString line = file.readLine().trimmed();
		if (!line.startsWith('#'))
			break;
		
		if (line.startsWith(mazeHeader))
			mazeName = line.mid(mazeHeader.size() + 1, line.size() - mazeHeader.size() - 2);
		else if (line.startsWith(subjectHeader))
			subjectName = line.mid(subjectHeader.size() + 1, line.size() - subjectHeader.size() - 2);
		else if (line.startsWith(dateHeader))
			timestamp = QDateTime::fromString(line.mid(dateHeader.size()));
	}
	
	if (subjectName.size() > 0 && mazeName.size() > 0 && timestamp.isValid())
	{
		QList<QStandardItem*> newRow;
		newRow.append(new QStandardItem(timestamp.toString("yyyy-MM-dd hh:mm:ss")));
		newRow.append(new QStandardItem(subjectName));
		newRow.append(new QStandardItem(mazeName));
		QStandardItem * const filePathItem = new QStandardItem(QDir::current().relativeFilePath(filename));
		newRow.append(filePathItem);
		for (QList<QStandardItem*>::iterator it = newRow.begin(); it != newRow.end(); ++it)
			(*it)->setEditable(false);
		appendRow(newRow);
		logfiles.insert(info.absoluteFilePath(), QPersistentModelIndex(indexFromItem(filePathItem)));
		watcher->addPath(info.absoluteFilePath());
	}
}
