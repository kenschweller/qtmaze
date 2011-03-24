#include "TrialTreeView.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QUrl>
#include <QModelIndex>
#include <QDragEnterEvent>
#include <QDropEvent>

TrialTreeView::TrialTreeView(QWidget *parent) : QTreeView(parent)
{
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
}

TrialTreeView::~TrialTreeView()
{
}
