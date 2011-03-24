#ifndef WALLS_H
#define WALLS_H

#include <QVector>
#include <QPoint>
class QFile;
class QGLWidget;

class Walls
{
public:
	enum WallFlags
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
	Walls();
	~Walls();
	
	bool operator==(const Walls &other) const;
	bool operator!=(const Walls &other) const;
	
	void resize(int w, int h);
	void clear();
	bool read(QFile &file);
	
	bool contains(int row, int col) const;
	bool contains(const QPoint &vertex) const;
	int at(int row, int col) const;
	int at(const QPoint &vertex) const;
	
	bool addWallBetweenVertices(const QPoint &a, const QPoint &b);
	void removeWallBetweenVertices(const QPoint &a, const QPoint &b);
	void removeWallBetweenTiles(const QPoint &a, const QPoint &b);
	
	void setContext(QGLWidget *newContext);
	void draw() const;
	void drawTops() const;
protected:
	int & at(int row, int col);
	int & at(const QPoint &vertex);
	int width;
	int height;
	QVector<int> walls;
	QGLWidget *context;
	unsigned int wallTexture;
};

#endif
