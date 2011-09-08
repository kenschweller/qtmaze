#ifndef TILES_H
#define TILES_H

#include <QVector>
#include <QPoint>
class Walls;
class QGLWidget;

class Tiles
{
public:
	Tiles();
	~Tiles();

	bool operator==(const Tiles &other) const;
	bool operator!=(const Tiles &other) const;

	void resize(int w, int h);
	void clear();
	void floodFill(int row, int col, const Walls &walls);
	void floodFill(const QPoint &tile, const Walls &walls);

	bool contains(int row, int col) const;
	bool contains(const QPoint &tile) const;
	bool at(int row, int col) const;
	bool at(const QPoint &tile) const;

	void setContext(QGLWidget *newContext);
	void drawGrid() const;
	void drawFloor() const;
	void drawCeiling() const;
protected:
	bool & at(int row, int col);
	bool & at(const QPoint &tile);
	int width;
	int height;
	QVector<bool> tiles;
	QGLWidget *context;
	unsigned int floorTexture;
	unsigned int ceilingTexture;
};

#endif
