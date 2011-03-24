#include "TrialTreeView.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QUrl>
#include <QModelIndex>
#include <QDragEnterEvent>
#include <QDropEvent>

TrialTreeView::TrialTreeView(QWidget *parent) : QTreeView(parent)
{
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
