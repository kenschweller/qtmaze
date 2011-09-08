#include "QtMaze.h"
#include "MazeWidget.h"
#include "NewDialog.h"
#include "PrefDialog.h"
#include "FilePane.h"
#include "ImagePane.h"
#include "MazePane.h"
#include "TrialPane.h"
#include "DataPane.h"
#include "engine/Path.h"

#include <QMenuBar>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QKeySequence>

#include <QFileDialog>

#include <QStyle>
#include <QMessageBox>
#include <QRegExpValidator>

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

	connect(trialPane, SIGNAL(canRunTrials(bool)), runTrialsAction, SLOT(setEnabled(bool)));
	connect(dataPane, SIGNAL(logActivated(const QString &, const QString &)), this, SLOT(slot_LoadLog(const QString &, const QString &)));

	connect(imageDock, SIGNAL(wallTextureChanged(const QString &)), mazeWidget3d, SLOT(slot_SetCurrentWallTexture(const QString &)));
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
	fullscreenAction = viewMenu->addAction("Fullscreen");
	fullscreenAction->setShortcut(QKeySequence("Alt+Return"));
	connect(fullscreenAction, SIGNAL(toggled(bool)), this, SLOT(slot_ViewFullscreen(bool)));
	QAction * const showFilePaneAction = viewMenu->addAction("Show File Pane", browserDock, SLOT(setVisible(bool)));
	QAction * const showImagePaneAction = viewMenu->addAction("Show Image Pane", imageDock, SLOT(setVisible(bool)));
	QAction * const showTrialPaneAction = viewMenu->addAction("Show Trial Pane", trialPane, SLOT(setVisible(bool)));
	QAction * const showDataPaneAction = viewMenu->addAction("Show Data Pane", dataPane, SLOT(setVisible(bool)));
	fullscreenAction->setCheckable(true);
	fullscreenAction->setChecked(windowState() & Qt::WindowFullScreen);
	showFilePaneAction->setCheckable(true);
	showImagePaneAction->setCheckable(true);
	showTrialPaneAction->setCheckable(true);
	showDataPaneAction->setCheckable(true);
	connect(browserDock, SIGNAL(visibilityChanged(bool)), showFilePaneAction, SLOT(setChecked(bool)));
	connect(imageDock, SIGNAL(visibilityChanged(bool)), showImagePaneAction, SLOT(setChecked(bool)));
	connect(trialPane, SIGNAL(visibilityChanged(bool)), showTrialPaneAction, SLOT(setChecked(bool)));
	connect(dataPane, SIGNAL(visibilityChanged(bool)), showDataPaneAction, SLOT(setChecked(bool)));
}

void QtMaze::_PopulateToolbars()
{
	QActionGroup * const modeGroup = new QActionGroup(this);
	modeGroup->setExclusive(true);
	toolbar->addSeparator();
	QAction * const wallPlacingModeAction = toolbar->addAction("Place Walls", mazeWidget3d, SLOT(slot_SwitchToWallPlacingMode()));
	QAction * const wallPaintingModeAction = toolbar->addAction("Paint Walls", mazeWidget3d, SLOT(slot_SwitchToWallPaintingMode()));
	wallPlacingModeAction->setCheckable(true);
	wallPaintingModeAction->setCheckable(true);
	QActionGroup * const wallModeGroup = new QActionGroup(this);
	wallModeGroup->setExclusive(true);
	wallModeGroup->addAction(wallPlacingModeAction);
	wallModeGroup->addAction(wallPaintingModeAction);
	wallPlacingModeAction->setChecked(true);
	toolbar->addSeparator();
	QAction * const editingModeAction = toolbar->addAction("Top-Down View", this, SLOT(slot_SwitchToEditingMode()));
	QAction * const overviewModeAction = toolbar->addAction("Perspective View", this, SLOT(slot_SwitchToOverviewMode()));
	mouselookModeAction = toolbar->addAction("First Person View", this, SLOT(slot_SwitchToMouselookMode()));
	toolbar->addSeparator();
	editingModeAction->setCheckable(true);
	overviewModeAction->setCheckable(true);
	mouselookModeAction->setCheckable(true);
	modeGroup->addAction(editingModeAction);
	modeGroup->addAction(overviewModeAction);
	modeGroup->addAction(mouselookModeAction);
	editingModeAction->setChecked(true);

	toolbar->addWidget(_participantName = new QLineEdit(this));
	_participantName->setFixedWidth(200);
	_participantName->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_ ]*"), this));
	_participantName->setPlaceholderText("[test subject name]");

	runTrialsAction = toolbar->addAction("Run Trials", this, SLOT(slot_Run()));
	testAction = toolbar->addAction("Test Map (no logging)", this, SLOT(slot_Test()));
	cancelAction = toolbar->addAction("Cancel", this, SLOT(slot_Cancel()));
	runTrialsAction->setEnabled(false);
	cancelAction->setEnabled(false);

	addToolBar(toolbar);

}
#include <cstdio>
void QtMaze::_CreateDocks()
{
	addDockWidget(Qt::LeftDockWidgetArea, browserDock = new FilePane(this));
	// browserDock->hide();
	addDockWidget(Qt::RightDockWidgetArea, imageDock = new ImagePane(this));

	connect(browserDock, SIGNAL(mapFileActivated(const QString &)), mazeWidget3d, SLOT(open(const QString &)));

	addDockWidget(Qt::LeftDockWidgetArea, trialPane = new TrialPane(this));
	addDockWidget(Qt::LeftDockWidgetArea, dataPane = new DataPane(this));
	// dataPane->hide();
}

void QtMaze::slot_FileNew()
{
	if (newDialog->exec() == QDialog::Accepted)
	{
		mazeWidget3d->reset(newDialog->_width->value(), newDialog->_height->value(), newDialog->_description->document()->toPlainText());
	}
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
		setWindowState(windowState() | Qt::WindowFullScreen);
	else
		setWindowState(windowState() & ~Qt::WindowFullScreen);
	toolbar->setVisible(!fullscreen);
	imageDock->setVisible(!fullscreen);
	browserDock->setVisible(!fullscreen);
	trialPane->setVisible(!fullscreen);
	dataPane->setVisible(!fullscreen);
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
	mazeWidget3d->setFocus();
	mouselookModeAction->setChecked(true);
}

void QtMaze::slot_Run()
{
	trialPane->restart();
	const QString nextFilename = trialPane->getNextMaze();
	if (nextFilename.size() == 0)
		return;

	mazeWidget3d->setParticipantName(_participantName->text());
	mazeWidget3d->setParticipating(true);
	mazeWidget3d->open(nextFilename, true);
	mazeWidget3d->setFocus();
	slot_SwitchToMouselookMode();
	_TrialsStarted();
}

void QtMaze::slot_Test()
{
	mazeWidget3d->setParticipantName(_participantName->text());
	mazeWidget3d->setParticipating(false);
	mazeWidget3d->restart(true);
	mazeWidget3d->setFocus();
	testing = true;
	slot_SwitchToMouselookMode();
	_TrialsStarted();
}

void QtMaze::slot_Cancel()
{
	_TrialsEnded();
	mazeWidget3d->restart(false);
}

void QtMaze::_TrialsStarted()
{
	runTrialsAction->setEnabled(false);
	testAction->setEnabled(false);
	cancelAction->setEnabled(true);
	fullscreenAction->setChecked(true);
	imageDock->setEnabled(false);
	browserDock->setEnabled(false);
	trialPane->setEnabled(false);
	dataPane->setEnabled(false);
}

void QtMaze::_TrialsEnded()
{
	runTrialsAction->setEnabled(true);
	testAction->setEnabled(true);
	cancelAction->setEnabled(false);
	fullscreenAction->setChecked(false);
	imageDock->setEnabled(true);
	browserDock->setEnabled(true);
	trialPane->setEnabled(true);
	dataPane->setEnabled(true);
}

void QtMaze::slot_MazeCompleted(const QString &filename)
{
	if (filename.size() == 0 || testing)
	{
		testing = false;
		mazeWidget3d->restart(false);
		_TrialsEnded();
		return;
	}
	const QString nextFilename = trialPane->getNextMaze();
	if (nextFilename.size() > 0)
	{
		fullscreenAction->setChecked(true);
		mazeWidget3d->open(nextFilename, true);
	}
	else
	{
		_TrialsEnded();
	}
}
// #include <cstdio>
void QtMaze::slot_LoadLog(const QString &logFilename, const QString &mazeFilename)
{
	// printf("mazeName = %s, logFile = %s\n", mazeFilename.toAscii().data(), logFilename.toAscii().data());
	const bool succeeded = mazeWidget3d->open(mazeFilename);
	if (!succeeded)
	{
		QMessageBox msg;
		msg.setText("Error opening maze file!");
		msg.exec();
	}

	Path path;
	path.load(logFilename);
	mazeWidget3d->addPath(path);
}
