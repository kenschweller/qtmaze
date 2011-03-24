#ifndef MAZE_H
#define MAZE_H

#include <QVector>
#include <QMap>
#include <QSet>
#include <QPoint>
#include <QString>
#include "Camera.h"
#include "Orientation.h"
#include "Pictures.h"
class QGLWidget;

uint qHash(const QPoint &p);

class Maze
{
public:
	enum Walls
	{
		NoWalls   = 0,
		NorthWall = (1 << 0),
		SouthWall = (1 << 1),
		WestWall  = (1 << 2),
		EastWall  = (1 << 3),
		HorizontalWall  =  EastWall | WestWall,
		VerticalWall    = NorthWall | SouthWall,
		WestTees     = VerticalWall | WestWall,
		EastTee      = VerticalWall | EastWall,
		NorthTee   = HorizontalWall | NorthWall,
		SouthTee   = HorizontalWall | SouthWall,
		NorthWestCorner = NorthWall | WestWall,
		NorthEastCorner = NorthWall | EastWall,
		SouthWestCorner = SouthWall | WestWall,
		SouthEastCorner = SouthWall | EastWall,
		Intersection = VerticalWall | HorizontalWall
	};
public:
	Maze();
	
	Maze & operator=(const Maze &other);
	bool operator==(const Maze &other) const;
	
	void reset(int newWidth, int newHeight);
	bool load(const QString &filename);
	bool save(const QString &filename) const;
	
	void setContext(QGLWidget *c);
	
	Camera getStartingCamera();
	float getWidth() const;
	float getHeight() const;
	Orientation getStartingOrientation() const;
	bool containsTile(const QPoint &tile) const;
	bool containsVertex(const QPoint &vertex) const;
	bool mapPointInGoalRadius(const QPoint &point) const;
	
	QPoint getNearestVertex(const QPoint &point) const;
	QPoint getNearestTile(const QPoint &point) const;
	Orientation getNearestOrientation(const QPoint &point) const;
	Pictures::Picture getPicture(const Orientation &orientation) const;
	
	void setStartingOrientation(const Orientation &orientation);
	
	bool connectVertices(const QPoint &a, const QPoint &b);
	void disconnectVertices(const QPoint &a, const QPoint &b);
	void removeWall(const QPoint &a, const QPoint &b);
	
	void addPicture(const Orientation &orientation, const QString &filename);
	void removePicture(const Orientation &orientation);
	
	void addGoal(const QPoint &p);
	void removeGoal(const QPoint &p);
	
	void startDroppingImage(const QString &filename);
	void moveDroppingImage(const QPoint &p);
	void endDroppingImage(bool dropped);
	
	void startDroppingGoal();
	void moveDroppingGoal(const QPoint &p);
	void endDroppingGoal(bool dropped);
	
	/*void startResizing();
	void moveResizing(const QPoint &p);
	void stopResizing();*/
	
	QPointF addDisplacement(const QPointF &position, QPointF displacement) const;
	
	void drawGrid();
	void draw();
protected:
	int & vertexAt(const QPoint &p) {return walls[p.y()*(width+1) + p.x()];}
	int & vertexAt(int row, int col) {return walls[row*(width+1) + col];}
	bool & tileAt(const QPoint &p) {return tiles[p.y()*width + p.x()];}
	bool & tileAt(int row, int col) {return tiles[row*width + col];}
	int vertexAt(const QPoint &p) const {return walls[p.y()*(width+1) + p.x()];}
	int vertexAt(int row, int col) const {return walls[row*(width+1) + col];}
	bool tileAt(int row, int col) const {return tiles[row*width + col];}
	// bool tileAt(int row, int col) const {return tiles[row*width + col];}
	
	void _RefreshTiles();
	void _DrawGoal(const QPoint &goal);
	void _DrawFloor();
	void _DrawCeiling();
	void _DrawEditorWallTops();
	void _DrawWalls();
	
	int width;
	int height;
	QVector<int>  walls;
	QVector<bool> tiles;
	Pictures pictures;
	Orientation startingOrientation;
	typedef QSet<QPoint> GoalSet;
	GoalSet goalTiles;
	class Dropping
	{
	public:
		Dropping() : dragging(DRAGGING_NONE) {}
		Orientation orientation;
		QString filename;
		enum DraggingWhich
		{
			DRAGGING_NONE,
			DRAGGING_IMAGE,
			DRAGGING_GOAL
		} dragging;
	} dropping;
	
	QGLWidget *context;
	unsigned int floorTexture;
	unsigned int ceilingTexture;
	unsigned int wallTexture;
};

#endif
