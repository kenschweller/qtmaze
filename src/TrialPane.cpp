#include "TrialPane.h"
#include "TrialTreeView.h"
#include "TrialTreeModel.h"

#include <QStandardItemModel>

TrialPane::TrialPane(QWidget *parent) : QDockWidget("Maze Queue", parent)
{
	TrialTreeView * const treeView = new TrialTreeView(this);
	setWidget(treeView);
	setFeatures(QDockWidget::DockWidgetClosable);
	
	treeView->setModel(_model = new TrialTreeModel(this));
}
