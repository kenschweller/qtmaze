#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QStandardItemModel>
#include <QMap>
#include <QString>
#include <QPersistentModelIndex>
class QFileSystemWatcher;
class QDir;

class DataModel : public QStandardItemModel
{
	Q_OBJECT
public:
	DataModel(QObject *parent = NULL);
	~DataModel();
protected slots:
	void slot_DirectoryChanged(const QString &dirPath);
	void slot_FileChanged(const QString &filePath);
protected:
	void _CrawlDirectory(const QDir &dir);
	void _AddLogFile(const QString &filename);

	QFileSystemWatcher *watcher;
	typedef QMap<QString, QPersistentModelIndex> FileMap;
	FileMap logfiles;
};

#endif
