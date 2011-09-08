#include "TrialPane.h"
#include "TrialTreeView.h"
#include "TrialTreeModel.h"

#include <QGridLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

TrialPane::TrialPane(QWidget *parent) : QDockWidget("Maze Queue", parent)
{
	QWidget * const dummy = new QWidget(this);
	QGridLayout * const grid = new QGridLayout(dummy);

	TrialTreeView * const treeView = new TrialTreeView(dummy);
	treeView->setModel(_model = new TrialTreeModel(this));\
	QPushButton * const newButton = new QPushButton("New", this);
	QPushButton * const openButton = new QPushButton("Open", this);
	saveButton = new QPushButton("Save", this);
	QPushButton * const saveAsButton = new QPushButton("Save As", this);
	grid->addWidget(newButton, 0, 0);
	grid->addWidget(openButton, 0, 1);
	grid->addWidget(saveButton, 0, 2);
	grid->addWidget(saveAsButton, 0, 3);
	grid->addWidget(treeView, 1, 0, 1, 4);

	setWidget(dummy);
	setFeatures(QDockWidget::DockWidgetClosable);

	connect(newButton, SIGNAL(clicked()), this, SLOT(slot_New()));
	connect(openButton, SIGNAL(clicked()), this, SLOT(slot_Open()));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(slot_Save()));
	connect(saveAsButton, SIGNAL(clicked()), this, SLOT(slot_SaveAs()));

	connect(_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(slot_ModelChanged()));
	connect(_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex, int)), this, SLOT(slot_ModelChanged()));
	connect(_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(slot_ModelChanged()));

	saveButton->setEnabled(false);
}

void TrialPane::slot_ModelChanged()
{
	saveButton->setEnabled(true);
	emit(canRunTrials(_model->rowCount() > 0));
}

void TrialPane::slot_New()
{
	_model->removeRows(0, _model->rowCount());
	_listFilename = "";
	saveButton->setEnabled(false);
}

void TrialPane::slot_Open()
{
	const QString filename = QFileDialog::getOpenFileName(this, "Open Maze Queue...", "", "Queue Files (*.run)");
	if (filename.size() == 0)
		return;

	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox msg;
		msg.setText("Error opening maze queue file '" + file.fileName() + "'");
		msg.exec();
		return;
	}

	slot_New();

	while (!file.atEnd())
	{
		const QString line = file.readLine().trimmed();
		_model->appendMazeFile(line);
	}

	_listFilename = filename;
}

void TrialPane::slot_Save()
{
	if (_listFilename.size() == 0)
		slot_SaveAs();
	else
		_Save(_listFilename);
}

void TrialPane::slot_SaveAs()
{
	const QString filename = QFileDialog::getSaveFileName(this, "Save Maze Queue As...", "", "Queue Files (*.run)");
	if (filename.size() == 0)
		return;
	_Save(filename);
}

void TrialPane::_Save(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		QMessageBox msg;
		msg.setText("Error saving maze queue file as '" + file.fileName() + "'");
		msg.exec();
		return;
		// return false;
	}

	QTextStream out(&file);
	{
		QModelIndex current = _model->index(0, 0);
		while (true)
		{
			const QString mazeFilename = _model->data(current, Qt::UserRole+1).toString();
			if (mazeFilename.size() == 0)
				break;
			out << mazeFilename << "\n";
			current = _model->sibling(current.row()+1, 0, current);
		}
	}
	_listFilename = filename;
	saveButton->setEnabled(false);
	// return true;
}

void TrialPane::restart()
{
	_nextMaze = _model->index(0, 0);
}

QString TrialPane::getNextMaze()
{
	if (!_nextMaze.isValid())
		return "";

	QString filename;
	bool exists = false;
	do
	{
		filename = _model->data(_nextMaze, Qt::UserRole+1).toString();
		QModelIndex tmp = _model->sibling(_nextMaze.row()+1, 0, _nextMaze);
		_nextMaze = tmp;
		exists = QFile::exists(filename);
		if (!exists)
		{
			QMessageBox msg;
			msg.setText("Error, maze file doesn't exist: '" + filename + "'");
			msg.exec();
		}
	} while (!exists && _nextMaze.isValid());

	if (!exists)
		return "";

	return filename;
}
