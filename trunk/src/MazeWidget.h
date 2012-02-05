#ifndef MAZEWIDGET3D_H
#define MAZEWIDGET3D_H

#include <QGLWidget>
#include <QTimer>
#include "engine/Maze.h"
#include "engine/Camera.h"
#include "engine/PositionLogger.h"
#include <QPoint>
#include <QPointF>
#include <QVector>
#include <QRect>
#include <QSize>
#include <QSound>
class Path;

#include <SDL.h>

class MazeWidget3D : public QGLWidget
{
	Q_OBJECT
public:
	MazeWidget3D(QWidget *parent = NULL);
	~MazeWidget3D();

	void reset(int width, int height/*, const QString description = QString()*/);
	bool save(const QString &filename);
	QString getFilename() const;
	QSize getMazeSize() const;
	int getHorizontalPageStep() const;
	int getVerticalPageStep() const;
	int getHorizontalOffset() const;
	int getVerticalOffset() const;

	bool isParticipating() const;

	void restart(bool pplaying);
	void levelView();

	void addPath(const Path &path);
	void clearPaths();
signals:
	void zoomChanged();
	void completedMaze(const QString &filename);
public slots:
	void setParticipantName(const QString &name);
	void setParticipating(bool enabled);
	bool open(const QString &filename, bool pplaying = false);

	void slot_SwitchToEditingMode();
	void slot_SwitchToOverviewMode();
	void slot_SwitchToMouselookMode();
	void slot_SwitchToWallPlacingMode();
	void slot_SwitchToWallPaintingMode();
	void slot_SwitchToWallHeightSettingMode();

	void slot_SetVerticalOffset(int x);
	void slot_SetHorizontalOffset(int x);

	void slot_SetJoystickDeadZone(double deadZone);
	void slot_SetJoystickWalkingSpeed(double walkingSpeed);
	void slot_SetJoystickTurningSpeed(double turningSpeed);

	void slot_SetCurrentWallTexture(const QString &filename);
	void slot_SetCurrentWallHeight(int newHeight);
protected slots:
	void slot_Advance();
	void slot_SoundFinished();
protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void focusOutEvent(QFocusEvent *event);
	void contextMenuEvent(QContextMenuEvent *event);

	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);

	void wheelEvent(QWheelEvent *event);

	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	float calculateAspectRatio() const;
	void _SetupProjectionMatrix();
	void _SetupModelMatrix();
	QPoint _Unproject(const QPoint &point, bool wallsToo = false);
	void _DrawCamera();
	void _RestartLogging();
	QVector3D _GetKeyboardDirection();
	QVector3D _GetJoystickDirection();
	float _GetKeyboardTurning();
	float _GetJoystickTurning();

	enum ViewMode
	{
		MODE_EDITING,
		MODE_OVERVIEW,
		MODE_MOUSELOOK
	} mode;
	enum Occupation
	{
		OCCUPATION_NONE,
		OCCUPATION_DRAWING,
		OCCUPATION_PAINTING,
		OCCUPATION_SETTINGWALLHEIGHT,
		OCCUPATION_MOUSELOOK,
		OCCUPATION_RESIZING
	} occupation;
	class Resizing
	{
	public:
		Resizing() : side(None)
		{
		}
		QRect newSize;
		enum Side
		{
			None,
			Top,
			Bottom,
			Right,
			Left
		} side;
	} resizing;

	QTimer timer;
	Maze maze;
	Maze oldMaze;
	QVector<Path> paths;
	Camera camera;
	Camera tumble;
	PositionLogger logger;
	QPoint lastPos;
	int holdingKeys[6];
	QVector<QPoint> drawPoints;
	QPoint lastTilePos;
	float scaleFactor;
	float editingScaleFactor;
	SDL_Joystick *joystick;
	QString _participantName;
	QString _filename;
	bool _participating;
	int verticalOffset;
	int horizontalOffset;
	int overviewVerticalOffset;
	int overviewHorizontalOffset;
	GLUquadric *quadric;
	unsigned int displayListMaze;
	QSound goalSound;
	bool playing;
	bool won;
	double joystickDeadZone;
	double joystickWalkingSpeed;
	double joystickTurningSpeed;
	QString currentWallTextureFilename;
	int currentWallHeight;
	enum BulldozingMode
	{
		BULLDOZING_PLACING_WALLS,
		BULLDOZING_PAINTING_WALLS,
		BULLDOZING_SETTING_WALL_HEIGHT
	} paintingWallsMode;
};

#endif
