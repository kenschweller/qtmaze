#include "TrialTreeView.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QUrl>
#include <QModelIndex>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMenu>

TrialTreeView::TrialTreeView(QWidget *parent) : QTreeView(parent)
{
	setRootIsDecorated(false);
	slot_SetEnabled(true);
}

TrialTreeView::~TrialTreeView()
{
}

void TrialTreeView::slot_SetEnabled(bool enabled)
{
	setDragEnabled(enabled);
	setAcceptDrops(enabled);
	setDropIndicatorShown(enabled);
}

void TrialTreeView::contextMenuEvent(QContextMenuEvent *event)
{
	const bool inViewport = viewport()->rect().contains(event->pos());
	const QModelIndex clickedIndex = indexAt(event->pos());
	if (!inViewport || !clickedIndex.isValid())
	{
		QTreeView::contextMenuEvent(event);
		return;
	}
	
	QMenu menu;
	QAction * const removeAction = menu.addAction("Remove");
	QAction * const result = menu.exec(event->globalPos());
	
	if (result == removeAction)
	{
		model()->removeRow(clickedIndex.row(), model()->parent(clickedIndex));
	}
}
