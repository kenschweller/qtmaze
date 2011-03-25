#include "QtMaze.h"
#include "MazeWidget.h"
#include "NewDialog.h"
#include "PrefDialog.h"
#include "FilePane.h"
#include "ImagePane.h"
#include "MazePane.h"
#include "TrialPane.h"

#include <QMenuBar>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QKeySequence>

#include <QFileDialog>

#include <QStyle>
#include <QMessageBox>

QtMaze::QtMaze(QWidget *parent) : QMainWindow(parent), settings(QSettings::UserScope, "forestdarling.com", "QtMaze"), testing(false)
{
	workingDirectory.setPath(settings.value("working_directory").toString());
	if (!restoreGeometry(settings.value("window_geometry").toByteArray()))
	{
		setGeometry(QRect(50, 50, 800, 600));
	}
	if (workingDirectory.path() == "" || !workingDirectory.exists())
	{
		workingDirectory = QDir::current();
	}

	setCentralWidget(mazePane = new MazePane(this));
	mazeWidget3d = mazePane->getMazeWidget3d();
	connect(mazeWidget3d, SIGNAL(completedMaze(const QString &)), this, SLOT(slot_MazeCompleted(const QString &)));

	toolbar = new QToolBar(this);
	toolbar->setFloatable(false);
	toolbar->setMovable(false);
	_CreateDocks();
	_CreateMenus();
	_PopulateToolbars();

	newDialog = new NewDialog(settings, this);
	prefDialog = new PrefDialog(settings, this);
	
	connect(prefDialog, SIGNAL(deadZoneChanged(double)), mazeWidget3d, SLOT(slot_SetJoystickDeadZone(double)));
	connect(prefDialog, SIGNAL(turningSpeedChanged(double)), mazeWidget3d, SLOT(slot_SetJoystickTurningSpeed(double)));
	connect(prefDialog, SIGNAL(walkingSpeedChanged(double)), mazeWidget3d, SLOT(slot_SetJoystickWalkingSpeed(double)));
	prefDialog->emitChanged(); // HACK, forces the prefDialog to emit signals to initially set the mazeWidget3d settings
}

QtMaze::~QtMaze()
{
	settings.setValue("working_directory", workingDirectory.absolutePath());
	settings.setValue("window_geometry", saveGeometry());
	newDialog->saveSettings(settings);
	prefDialog->saveSettings(settings);
}

void QtMaze::_CreateMenus()
{
	QMenu * const fileMenu = menuBar()->addMenu("&File");
	toolbar->addAction(fileMenu->addAction(style()->standardIcon(QStyle::SP_FileIcon), "New", this, SLOT(slot_FileNew()), QKeySequence("Ctrl+N")));
	toolbar->addAction(fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Open", this, SLOT(slot_FileOpen()), QKeySequence("Ctrl+O")));
	toolbar->addAction(fileMenu->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Save", this, SLOT(slot_FileSave()), QKeySequence("Ctrl+S")));
	fileMenu->addAction("Save As...", this, SLOT(slot_FileSaveAs()), QKeySequence("Ctrl+Alt+S"));
	fileMenu->addAction("Close");
	fileMenu->addSeparator();
	fileMenu->addAction("Exit", this, SLOT(close()));

	QMenu * const editMenu = menuBar()->addMenu("&Edit");
	editMenu->addAction("Undo");
	editMenu->addAction("Redo");
	editMenu->addSeparator();
	editMenu->addAction("Preferences", this, SLOT(slot_EditPreferences()));

	QMenu * const viewMenu = menuBar()->addMenu("&View");
	QAction * const fullscreenAction = viewMenu->addAction("Fullscreen", this, SLOT(slot_ViewFullscreen(bool)), QKeySequence("Alt+Return"));
	QAction * const showFilePaneAction = viewMenu->addAction("Show File Pane", browserDock, SLOT(setVisible(bool)));
	QAction * const showImagePaneAction = viewMenu->addAction("Show Image Pane", imageDock, SLOT(setVisible(bool)));
	QAction * const showTrialPaneAction = viewMenu->addAction("Show Trial Pane", trialPane, SLOT(setVisible(bool)));
	fullscreenAction->setCheckable(true);
	showFilePaneAction->setCheckable(true);
	showImagePaneAction->setCheckable(true);
	showTrialPaneAction->setCheckable(true);
	connect(browserDock, SIGNAL(visibilityChanged(bool)), showFilePaneAction, SLOT(setChecked(bool)));
	connect(imageDock, SIGNAL(visibilityChanged(bool)), showImagePaneAction, SLOT(setChecked(bool)));
	connect(trialPane, SIGNAL(visibilityChanged(bool)), showTrialPaneAction, SLOT(setChecked(bool)));
}

void QtMaze::_PopulateToolbars()
{
	QActionGroup * modeGroup = new QActionGroup(this);
	modeGroup->setExclusive(true);
	toolbar->addSeparator();
	QAction * const editingModeAction = toolbar->addAction("Editing Mode", this, SLOT(slot_SwitchToEditingMode()));
	QAction * const overviewModeAction = toolbar->addAction("Overview Mode", this, SLOT(slot_SwitchToOverviewMode()));
	QAction * const mouselookModeAction = toolbar->addAction("Mouselook Mode", this, SLOT(slot_SwitchToMouselookMode()));
	toolbar->addSeparator();
	editingModeAction->setCheckable(true);
	overviewModeAction->setCheckable(true);
	mouselookModeAction->setCheckable(true);
	modeGroup->addAction(editingModeAction);
	modeGroup->addAction(overviewModeAction);
	modeGroup->addAction(mouselookModeAction);
	// toolbar->addSeparator();
	toolbar->addWidget(_participantName = new QLineEdit(this));
	// _participantName->setEditable(true);
	_participantName->setFixedWidth(200);
	toolbar->addAction("Run Trials", this, SLOT(slot_Run()));
	toolbar->addAction("Test Map (no logging)", this, SLOT(slot_Test()));
	addToolBar(toolbar);

	editingModeAction->setChecked(true);
}
#include <cstdio>
void QtMaze::_CreateDocks()
{
	addDockWidget(Qt::LeftDockWidgetArea, browserDock = new FilePane(this));
	// browserDock->hide();
	addDockWidget(Qt::RightDockWidgetArea, imageDock = new ImagePane(this));

	connect(browserDock, SIGNAL(mapFileActivated(const QString &)), mazeWidget3d, SLOT(open(const QString &)));

	addDockWidget(Qt::LeftDockWidgetArea, trialPane = new TrialPane(this));
}

void QtMaze::keyPressEvent(QKeyEvent *event)
{
	printf("Whoa\n");
}

void QtMaze::slot_FileNew()
{
	// TODO
	// HACK
	// mazeWidget3d->releaseKeyboard();
	if (newDialog->exec() == QDialog::Accepted)
	{
		mazeWidget3d->reset(newDialog->_width->value(), newDialog->_height->value(), newDialog->_description->document()->toPlainText());
	}
	// mazeWidget3d->grabKeyboard();
}

void QtMaze::slot_FileOpen()
{
	const QString filename = QFileDialog::getOpenFileName(this, "Open Maze File...", "", "Maze Files (*.map)");
	if (filename.size() > 0)
	{
		const bool succeeded = mazeWidget3d->open(filename);
		if (!succeeded)
		{
			QMessageBox msg;
			msg.setText("Error opening maze file!");
			msg.exec();
		}
	}
}

void QtMaze::slot_FileSave()
{
	const QString filename = mazeWidget3d->getFilename();
	if (filename.size() > 0)
	{
		if (!mazeWidget3d->save(filename))
		{
			QMessageBox msg;
			msg.setText("Error saving maze file!");
			msg.exec();
		}
	}
	else
		slot_FileSaveAs();

}

void QtMaze::slot_FileSaveAs()
{
	const QString filename = QFileDialog::getSaveFileName(this, "Save Maze File As...", "", "Maze Files (*.map)");
	if (filename.size() > 0)
	{
		if (!mazeWidget3d->save(filename))
		{
			QMessageBox msg;
			msg.setText("Error saving maze file!");
			msg.exec();
		}
	}
}

void QtMaze::slot_EditPreferences()
{
	prefDialog->show();
}

void QtMaze::slot_ViewFullscreen(bool fullscreen)
{
	if (fullscreen)
	{
		setWindowState(windowState() | Qt::WindowFullScreen);
		toolbar->hide();
		imageDock->hide();
		browserDock->hide();
		trialPane->hide();
	}
	else
	{
		setWindowState(windowState() & ~Qt::WindowFullScreen);
		toolbar->show();
		imageDock->show();
		browserDock->show();
		trialPane->show();
	}
}

void QtMaze::slot_SwitchToEditingMode()
{
	mazePane->showScrollbars();
	mazeWidget3d->slot_SwitchToEditingMode();
	mazePane->slot_RefreshScrollbars();
}

void QtMaze::slot_SwitchToOverviewMode()
{
	mazePane->showScrollbars();
	mazeWidget3d->slot_SwitchToOverviewMode();
	mazePane->slot_RefreshScrollbars();
}

void QtMaze::slot_SwitchToMouselookMode()
{
	mazePane->hideScrollbars();
	mazeWidget3d->slot_SwitchToMouselookMode();
}

void QtMaze::slot_Run()
{
	trialPane->restart();
	const QString nextFilename = trialPane->getNextMaze();
	if (nextFilename.size() == 0)
		return;
	
	mazeWidget3d->setParticipantName(_participantName->text());
	mazeWidget3d->setParticipating(true);
	// mazeWidget3d->restart(true);
	slot_SwitchToMouselookMode();
	mazeWidget3d->open(nextFilename, true);
}

void QtMaze::slot_Test()
{
	mazeWidget3d->setParticipantName(_participantName->text());
	mazeWidget3d->setParticipating(false);
	mazeWidget3d->restart(true);
	testing = true;
	slot_SwitchToMouselookMode();
}

void QtMaze::slot_MazeCompleted(const QString &filename)
{
	if (filename.size() == 0 || testing)
	{
		testing = false;
		mazeWidget3d->restart(false);
		return;
	}
	const QString nextFilename = trialPane->getNextMaze();
	if (nextFilename.size() > 0)
		mazeWidget3d->open(nextFilename, true);
}
