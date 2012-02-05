#ifndef WALLS_H
#define WALLS_H

#include "Orientation.h"
#include <QVector>
#include <QMap>
#include <QPair>
#include <QPoint>
class QFile;
class QGLWidget;
#include "WallTextures.h"

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
	struct TextureIDs
	{
		TextureIDs() : vertex(0), east_northeast(0), east_southeast(0), south_southwest(0), south_southeast(0) {}
		bool operator==(const TextureIDs &other) const {return vertex == other.vertex && east_northeast == other.east_northeast && east_southeast == other.east_southeast && south_southwest == other.south_southwest && south_southeast == other.south_southeast;}
		bool operator!=(const TextureIDs &other) const {return vertex != other.vertex || east_northeast != other.east_northeast || east_southeast != other.east_southeast || south_southwest != other.south_southwest || south_southeast != other.south_southeast;}
		quint8 vertex;
		quint8 east_northeast;
		quint8 east_southeast;
		quint8 south_southwest;
		quint8 south_southeast;
	};
public:
	Walls();
	~Walls();

	const WallTextures & wallTextures() const {return textures;}

	bool operator==(const Walls &other) const;
	bool operator!=(const Walls &other) const;

	void resize(int w, int h);
	void clear();
	bool read(QFile &file);
	bool readTextures(QFile &file);
	bool readHeightmap(QFile &file);

	bool contains(int row, int col) const;
	bool contains(const QPoint &vertex) const;
	int at(int row, int col) const;
	int at(const QPoint &vertex) const;
	const TextureIDs & internalTextureIdAt(int row, int col) const;
	const TextureIDs & internalTextureIdAt(const QPoint &vertex) const;

	bool addWallBetweenVertices(const QPoint &a, const QPoint &b);
	void removeWallBetweenVertices(const QPoint &a, const QPoint &b);
	void removeWallBetweenTiles(const QPoint &a, const QPoint &b);
	void paintWall(const Orientation &p, const QString &filename);
	void setWallHeight(const Orientation &p, const int newHeight);

	void setContext(QGLWidget *newContext);
	void draw() const;
	void drawTops() const;
protected:
	void _AddTexture(const QString &filename);

	int & at(int row, int col);
	int & at(const QPoint &vertex);
	TextureIDs & internalTextureIdAt(int row, int col);
	TextureIDs & internalTextureIdAt(const QPoint &vertex);
	int width;
	int height;
	QVector<int> walls;
	QVector<TextureIDs> texturemap;
	QVector<TextureIDs> heightmap;
	WallTextures textures;

	friend class Maze;
};

#endif
