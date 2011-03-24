#ifndef FILEPANE_H
#define FILEPANE_H

#include <QDockWidget>
class QModelIndex;
class QFileSystemModel;
class QTreeView;

class FilePane : public QDockWidget
{
	Q_OBJECT
public:
	FilePane(QWidget *parent = NULL);
signals:
	void mapFileActivated(const QString &filename);
protected slots:
	void slot_Activated(const QModelIndex &index);
protected:
	QFileSystemModel *fileModel;
	QTreeView *treeView;
};

#endif
