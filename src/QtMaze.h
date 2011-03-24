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
class FilePane;
class TrialPane;
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
	void slot_ViewFullscreen(bool fullscreen);
	
	void slot_SwitchToEditingMode();
	void slot_SwitchToOverviewMode();
	void slot_SwitchToMouselookMode();
	
	void slot_Run();
	void slot_Test();
	void slot_MazeCompleted(const QString &filename);
private:
	void _CreateMenus();
	void _PopulateToolbars();
	void _CreateDocks();
protected:
	void keyPressEvent(QKeyEvent *event);
	
	QSettings settings;
	QDir workingDirectory;
	QToolBar *toolbar;
	FilePane *browserDock; 
	ImagePane *imageDock; 
	MazePane *mazePane;
	TrialPane *trialPane;
	MazeWidget3D *mazeWidget3d;
	NewDialog *newDialog;
	QLineEdit *_participantName;
	bool testing;
};

#endif
