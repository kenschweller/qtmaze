#include "MazePane.h"
#include "MazeWidget.h"

#include <QGridLayout>
#include <QScrollBar>

MazePane::MazePane(QWidget *parent) : QWidget(parent), _mazeWidget3d(NULL)
{
	_mazeWidget3d = new MazeWidget3D(this);
	
	_verticalScrollBar = new QScrollBar(Qt::Vertical, this);
	_horizontalScrollBar = new QScrollBar(Qt::Horizontal, this);
	
	QGridLayout * grid = new QGridLayout(this);
	grid->setContentsMargins(0, 0, 0, 0);
	grid->setSpacing(0);
	grid->addWidget(_mazeWidget3d, 0, 0);
	grid->addWidget(_verticalScrollBar, 0, 1);
	grid->addWidget(_horizontalScrollBar, 1, 0);
	
	connect(_verticalScrollBar, SIGNAL(valueChanged(int)), _mazeWidget3d, SLOT(slot_SetVerticalOffset(int)));
	connect(_horizontalScrollBar, SIGNAL(valueChanged(int)), _mazeWidget3d, SLOT(slot_SetHorizontalOffset(int)));
	connect(_mazeWidget3d, SIGNAL(zoomChanged()), this, SLOT(slot_RefreshScrollbars()));
	
	slot_RefreshScrollbars();
}
// #include <cstdio>
void MazePane::slot_RefreshScrollbars()
{
	const QSize mazeSize = _mazeWidget3d->getMazeSize();
	const int horizPageStep = _mazeWidget3d->getVerticalPageStep();
	const int vertPageStep = _mazeWidget3d->getHorizontalPageStep();
	
	// printf("horizPageStep = %i\n", horizPageStep);
	// printf("vertPageStep = %i\n", vertPageStep);
	
	_horizontalScrollBar->setMinimum(0);
	_horizontalScrollBar->setMaximum(mazeSize.width());
	_horizontalScrollBar->setPageStep(horizPageStep);
	_horizontalScrollBar->setValue(_mazeWidget3d->getHorizontalOffset());
	_verticalScrollBar->setMinimum(0);
	_verticalScrollBar->setMaximum(mazeSize.height());
	_verticalScrollBar->setPageStep(vertPageStep);
	_verticalScrollBar->setValue(_mazeWidget3d->getVerticalOffset());
}

void MazePane::showScrollbars()
{
	_verticalScrollBar->setEnabled(true);
	_horizontalScrollBar->setEnabled(true);
}

void MazePane::hideScrollbars()
{
	_verticalScrollBar->setEnabled(false);
	_horizontalScrollBar->setEnabled(false);
}

void MazePane::resizeEvent(QResizeEvent *event)
{
	slot_RefreshScrollbars();
}
