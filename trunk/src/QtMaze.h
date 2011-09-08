#ifndef QTMAZE_H
#define QTMAZE_H

#include <QMainWindow>
#include <QSettings>
#include <QDir>
class QToolBar;
class QAction;
class QLineEdit;
class MazeWidget3D;
class NewDialog;
class PrefDialog;
class FilePane;
class TrialPane;
class DataPane;
class ImagePane;
class MazePane;

class QtMaze : public QMainWindow
{
	Q_OBJECT
public:
	QtMaze(QWidget *parent = NULL);
	~QtMaze();
protected slots:
	void slot_FileNew();
	void slot_FileOpen();
	void slot_FileSave();
	void slot_FileSaveAs();
	void slot_EditPreferences();
	void slot_ViewFullscreen(bool fullscreen);

	void slot_SwitchToEditingMode();
	void slot_SwitchToOverviewMode();
	void slot_SwitchToMouselookMode();

	void slot_Run();
	void slot_Test();
	void slot_Cancel();
	void slot_MazeCompleted(const QString &filename);
	void slot_LoadLog(const QString &logFilename, const QString &mazeFilename);
private:
	void _CreateMenus();
	void _PopulateToolbars();
	void _CreateDocks();
	void _TrialsStarted();
	void _TrialsEnded();
protected:
	QSettings settings;
	QDir workingDirectory;
	QToolBar *toolbar;
	FilePane *browserDock;
	ImagePane *imageDock;
	MazePane *mazePane;
	TrialPane *trialPane;
	DataPane *dataPane;
	MazeWidget3D *mazeWidget3d;
	NewDialog *newDialog;
	PrefDialog *prefDialog;
	QLineEdit *_participantName;
	QAction *fullscreenAction;
	QAction *mouselookModeAction;
	QAction *runTrialsAction;
	QAction *testAction;
	QAction *cancelAction;
	bool testing;
};

#endif
