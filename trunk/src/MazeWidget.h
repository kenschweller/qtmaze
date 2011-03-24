#ifndef MAZEWIDGET3D_H
#define MAZEWIDGET3D_H

#include <QGLWidget>
#include <QTimer>
#include "engine/Maze.h"
#include "engine/Camera.h"
#include "engine/PositionLogger.h"
#include <QPoint>
#include <QVector>
#include <QRect>
#include <QSize>
#include <QSound>

#include <SDL.h>

class MazeWidget3D : public QGLWidget
{
	Q_OBJECT
public:
	MazeWidget3D(QWidget *parent = NULL);
	~MazeWidget3D();
	
	void reset(int width, int height, const QString description = QString());
	bool save(const QString &filename);
	QString getFilename() const;
	QSize getMazeSize() const;
	int getHorizontalPageStep() const;
	int getVerticalPageStep() const;
	int getHorizontalOffset() const;
	int getVerticalOffset() const;
	
	bool isParticipating() const;
	
	void restart(bool pplaying);
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
	
	void slot_SetVerticalOffset(int x);
	void slot_SetHorizontalOffset(int x);
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
	
	void wheelEvent(QWheelEvent *event);
	
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	
	float calculateAspectRatio() const;
	void _SetupProjectionMatrix();
	void _SetupModelMatrix();
	QPoint _Unproject(const QPoint &point);
	void _DrawCamera();
	void _RestartLogging();
	
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
};

#endif
