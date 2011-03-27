#include "DataPane.h"
#include "DataModel.h"

#include <QTableView>
#include <QHeaderView>
#include <QModelIndex>

DataPane::DataPane(QWidget *parent) : QDockWidget("Collected Data", parent)
{
	QTableView * const _tableView = new QTableView(this);
	_tableView->setSortingEnabled(true);
	_tableView->setCornerButtonEnabled(false);
	_tableView->verticalHeader()->hide();
	_tableView->horizontalHeader()->setHighlightSections(false);
	_tableView->horizontalHeader()->setStretchLastSection(true);
	_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	setWidget(_tableView);
	
	_model = new DataModel(this);
	_tableView->setModel(_model);
	
	_tableView->hideColumn(3);
	_tableView->resizeColumnsToContents();
	_tableView->resizeRowsToContents();
	_tableView->sortByColumn(0, Qt::DescendingOrder);
	
	connect(_tableView, SIGNAL(activated(const QModelIndex &)), this, SLOT(slot_Activated(const QModelIndex &)));
}

DataPane::~DataPane()
{
}

void DataPane::slot_Activated(const QModelIndex &index)
{
	if (!index.isValid())
		return;
	
	const QString mazeFilename = _model->data(_model->index(index.row(), 2)).toString();
	const QString logFilename = _model->data(_model->index(index.row(), 3)).toString();
	emit(logActivated(logFilename, mazeFilename));
}
