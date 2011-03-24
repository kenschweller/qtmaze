#include "TrialTreeView.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QUrl>
#include <QModelIndex>
#include <QDragEnterEvent>
#include <QDropEvent>

TrialTreeView::TrialTreeView(QWidget *parent) : QTreeView(parent)
{
	setAcceptDrops(true);
	setDropIndicatorShown(true);
}

TrialTreeView::~TrialTreeView()
{
}
/*#include <cstdio>
void TrialTreeView::dragEnterEvent(QDragEnterEvent *event)
{
	// QStringList formats = event->mimeData()->formats();
	// for (QStringList::const_iterator it = formats.begin(); it != formats.end(); ++it)
		// printf("Format: '%s'\n", it->toAscii().data());
	const QMimeData * const mimeData = event->mimeData();
	if (mimeData->hasUrls())
		event->accept();
}

void TrialTreeView::dropEvent(QDropEvent *event)
{
	QStandardItemModel * const mymodel = dynamic_cast<QStandardItemModel*>(model());
	QStringList formats = event->mimeData()->formats();
	for (QStringList::const_iterator it = formats.begin(); it != formats.end(); ++it)
		printf("Format: '%s'\n", it->toAscii().data());
	if (!mymodel)
		return;
	const QMimeData * const mimeData = event->mimeData();
	event->acceptProposedAction();
	if (mimeData->hasUrls())
	{
		const QList<QUrl> urls = mimeData->urls();
		const QModelIndex dropIntoIndex = indexAt(event->pos());
		QStandardItem * const dropInto = mymodel->itemFromIndex(dropIntoIndex);
		int i = 0;
		for (QList<QUrl>::const_iterator it = urls.begin(); it != urls.end(); ++it, i++)
		{
			// dropInto->appendRow(new QStandardItem(it->path()));
		}
	}
}
*/