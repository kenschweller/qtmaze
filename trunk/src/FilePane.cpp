#include "FilePane.h"

#include <QDir>
#include <QFileSystemModel>
#include <QTreeView>

FilePane::FilePane(QWidget *parent) : QDockWidget("Maze File Browser", parent), fileModel(NULL), treeView(NULL)
{
	fileModel = new QFileSystemModel(this);
	// fileModel->setRootPath(workingDirectory.absolutePath());
	fileModel->setSupportedDragActions(Qt::CopyAction);
	fileModel->setRootPath(QDir::currentPath());
	fileModel->setNameFilterDisables(false);
	fileModel->setFilter(QDir::Drives | QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
	// fileModel->setFilter(QDir::Files | QDir::AllDirs);
	QStringList filters;
	filters.append("*.map");
	fileModel->setNameFilters(filters);

	treeView = new QTreeView(this);
	treeView->setDragEnabled(true);
	treeView->setModel(fileModel);
	treeView->hideColumn(1);
	treeView->hideColumn(2);
	treeView->hideColumn(3);
	{
		QModelIndex index = fileModel->index(QDir::currentPath());
		while (index.isValid())
		{
			treeView->expand(index);
			index = fileModel->parent(index);
		}
	}

	setWidget(treeView);
	setFeatures(QDockWidget::DockWidgetClosable);

	connect(treeView, SIGNAL(activated(const QModelIndex &)), this, SLOT(slot_Activated(const QModelIndex &)));
}

void FilePane::slot_Activated(const QModelIndex &index)
{
	const QFileInfo info = fileModel->fileInfo(index);
	if (info.suffix().toLower() == "map")
		emit(mapFileActivated(info.absoluteFilePath()));
}
