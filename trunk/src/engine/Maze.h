#ifndef MAZE_H
#define MAZE_H

#include <QMap>
#include <QSet>
#include <QPoint>
#include <QString>
#include "Camera.h"
#include "Orientation.h"
#include "Pictures.h"
#include "Walls.h"
#include "Tiles.h"
class QGLWidget;

uint qHash(const QPoint &p);

class Maze
{
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
	Orientation getStartingOrientation() const {return startingOrientation;}
	bool containsTile(const QPoint &tile) const {return tiles.contains(tile);}
	bool containsVertex(const QPoint &vertex) const {return walls.contains(vertex);}
	bool mapPointInGoalRadius(const QPoint &point) const;

	QPoint getNearestVertex(const QPoint &point) const;
	QPoint getNearestTile(const QPoint &point) const;
	Orientation getNearestOrientation(const QPoint &point) const;
	Pictures::Picture getPicture(const Orientation &orientation) const {return pictures.at(orientation);}

	void setStartingOrientation(const Orientation &orientation);

	bool connectVertices(const QPoint &a, const QPoint &b);
	void disconnectVertices(const QPoint &a, const QPoint &b);
	void removeWall(const QPoint &a, const QPoint &b);

	void addPicture(const Orientation &orientation, const QString &filename);
	void removePicture(const Orientation &orientation) {pictures.remove(orientation);}

	void addGoal(const QPoint &p);
	void removeGoal(const QPoint &p) {goalTiles.remove(p);}

	void startDroppingImage(const QString &filename);
	void moveDroppingImage(const QPoint &p);
	void endDroppingImage(bool dropped);

	void startDroppingGoal();
	void moveDroppingGoal(const QPoint &p);
	void endDroppingGoal(bool dropped);

	void startPaintingWallTexture(const QString &filename);
	void continuePaintingWallTexture(const QPoint &p);
	void endPaintingWallTexture(bool dropped);

	/*void startResizing();
	void moveResizing(const QPoint &p);
	void stopResizing();*/

	QPointF addDisplacement(const QPointF &position, QPointF displacement) const;

	void drawGrid() const;
	void draw() const;
protected:
	void _RefreshTiles();
	void _DrawStartingOrientation() const;
	void _DrawGoals() const;
	void _DrawGoal(const QPoint &goal) const;

	int width;
	int height;
	Walls walls;
	Tiles tiles;
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
			DRAGGING_GOAL,
			DRAGGING_WALLTEXTURE
		} dragging;
	} dropping;

	QGLWidget *context;
};

#endif
