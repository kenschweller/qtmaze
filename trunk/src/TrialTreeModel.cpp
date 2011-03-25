#include "TrialTreeModel.h"

#include <QMimeData>
#include <QModelIndex>
#include <QUrl>
#include <QDir>

TrialTreeModel::TrialTreeModel(QObject *parent) : QStandardItemModel(parent)
{
	// appendRow(new QStandardItem("Test 1"));
	// appendRow(new QStandardItem("Test 2"));
	// appendRow(new QStandardItem("Test 3"));
	
	setColumnCount(1);
	setHeaderData(0, Qt::Horizontal, "Maze File");
	setSupportedDragActions(Qt::MoveAction);
}

TrialTreeModel::~TrialTreeModel()
{
}

QStringList TrialTreeModel::mimeTypes() const
{
	QStringList result = QStandardItemModel::mimeTypes();
	result.append("text/uri-list");
	return result;
}

void TrialTreeModel::appendMazeFile(const QString &filename)
{
	QFileInfo fileInfo(filename);
	QStandardItem * const newItem = new QStandardItem(fileInfo.fileName());
	newItem->setData(filename);
	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled/* | Qt::ItemIsDropEnabled*/);
	appendRow(newItem);
}

bool TrialTreeModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (QStandardItemModel::dropMimeData(mimeData, action, row, column, parent))
		return true;
	
	if (mimeData->hasUrls())
	{
		const QList<QUrl> urls = mimeData->urls();
		QStandardItem * dropInto = itemFromIndex(parent);
		if (dropInto && !dropInto->data().isNull())
		{
			// printf("Dropping onto '%s'\n", dropInto->data().toString().toAscii().data());
			QStandardItem * const tmp = new QStandardItem("Group");
			QStandardItem * const p = (dropInto->parent() ? dropInto->parent() : invisibleRootItem());
			p->insertRow(dropInto->row(), tmp);
			tmp->setChild(0, 0, dropInto->clone());
			p->removeRow(dropInto->row());
			dropInto = tmp;
		}
		if (!dropInto)
			dropInto = invisibleRootItem();
		int i = 0;
		for (QList<QUrl>::const_iterator it = urls.begin(); it != urls.end(); ++it, i++)
		{
			const QString filePath = it->path();
			QFileInfo fileInfo(filePath);
			QStandardItem * const newItem = new QStandardItem(fileInfo.fileName());
			newItem->setData(filePath);
			newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled/* | Qt::ItemIsDropEnabled*/);
			dropInto->appendRow(newItem);
		}
		return true;
	}
	return false;
}
