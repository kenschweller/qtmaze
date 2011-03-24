#include "TrialTreeModel.h"

#include <QMimeData>
#include <QModelIndex>
#include <QUrl>
#include <QDir>

TrialTreeModel::TrialTreeModel(QObject *parent) : QStandardItemModel(parent)
{
	appendRow(new QStandardItem("Test"));
}

TrialTreeModel::~TrialTreeModel()
{
}

Qt::DropActions TrialTreeModel::supportedDragActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

QStringList TrialTreeModel::mimeTypes() const
{
	QStringList result = QStandardItemModel::mimeTypes();
	result.append("text/uri-list");
	// return QStringList("text/uri-list");
	return result;
}

bool TrialTreeModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (mimeData->hasUrls())
	{
		const QList<QUrl> urls = mimeData->urls();
		QStandardItem * dropInto = itemFromIndex(parent);
		if (!dropInto)
			dropInto = invisibleRootItem();
		int i = 0;
		for (QList<QUrl>::const_iterator it = urls.begin(); it != urls.end(); ++it, i++)
		{
			// const QString txt = QDir::current().relativeFilePath(it->path());
			const QString txt = it->path();
			dropInto->appendRow(new QStandardItem(txt));
		}
		return true;
	}
	return false;
}
