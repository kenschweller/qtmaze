#include "Walls.h"
#include "defines.h"

#include <QFile>
#include <QString>
#include <QGLWidget>

// #include "GLee.h"

Walls::Walls() : width(0), height(0)
{
}

Walls::~Walls()
{
}

bool Walls::operator==(const Walls &other) const
{
	return walls == other.walls && texturemap == other.texturemap;
}

bool Walls::operator!=(const Walls &other) const
{
	return walls != other.walls || texturemap != other.texturemap;
}

void Walls::resize(int w, int h)
{
	width = w + 1;
	height = h + 1;
	walls.resize(width*height);
	texturemap.resize(width*height);
	heightmap.resize(width*height);
}

void Walls::clear()
{
	walls.fill(NoWalls);
	texturemap.fill(TextureIDs());
	heightmap.fill(Heights());
	textures.clear();
}

bool Walls::read(QFile &file)
{
	for (int row = 0; row < height - 1; row++)
	{
		if (!file.canReadLine())
			return false;
		const QString line = file.readLine().trimmed();
		if (line.size() < width - 1)
			return false;
		for (int col = 0; col < width; col++) // TODO should this be width - 1?
		{
			const QChar c = line[col];
			if (c == '+' || c == '-')
				walls[row*width+col] |= EastWall;
			if (c == '+' || c == '|')
				walls[row*width+col] |= SouthWall;
		}
	}

	// make the wall connections bidirectional
	for (int row = 1; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (at(row-1, col) & Walls::SouthWall)
				at(row, col) |= Walls::NorthWall;
		}
	}
	for (int row = 0; row < height; row++)
	{
		for (int col = 1; col < width; col++)
		{
			if (at(row, col-1) & Walls::EastWall)
				at(row, col) |= Walls::WestWall;
		}
	}

	return true;
}
// #include <QDebug>
bool Walls::readTextures(QFile &file)
{
	for (int row = 0; row < height; row++)
	{
		if (!file.canReadLine())
			return false;
		const QString line = file.readLine().trimmed();
		// qDebug() << line.size() << "--" << width*(2*5);
		if (line.size() < width*(2*5))
			return false;
		for (int col = 0; col < width; col++)
		{
			quint8 ids[5];
			for (int i = 0; i < 5; i++)
			{
				const QString numberString = line.mid(col*(2*5) + i*2, 2);
				bool ok = false;
				ids[i] = numberString.toUInt(&ok, 16);
				if (!ok)
					return false;
			}
			texturemap[row*width + col].vertex          = ids[0];
			texturemap[row*width + col].east_northeast  = ids[1];
			texturemap[row*width + col].east_southeast  = ids[2];
			texturemap[row*width + col].south_southwest = ids[3];
			texturemap[row*width + col].south_southeast = ids[4];
		}
	}

	return true;
}

bool Walls::readHeightmap(QFile &file)
{
	for (int row = 0; row < height; row++)
	{
		if (!file.canReadLine())
			return false;
		const QString line = file.readLine().trimmed();
		if (line.size() < width*(2*4))
			return false;
		for (int col = 0; col < width; col++)
		{
			quint8 ids[4];
			for (int i = 0; i < 4; i++)
			{
				const QString numberString = line.mid(col*(2*4) + i*2, 2);
				bool ok = false;
				ids[i] = numberString.toUInt(&ok, 16);
				if (!ok)
					return false;
			}
			heightmap[row*width + col].north = ids[0];
			heightmap[row*width + col].south = ids[1];
			heightmap[row*width + col].east  = ids[2];
			heightmap[row*width + col].west  = ids[3];
		}
	}
	return true;
}

bool Walls::contains(int row, int col) const
{
	return row >= 0 && col >= 0 && row < height && col < width;
}

bool Walls::contains(const QPoint &vertex) const
{
	return contains(vertex.y(), vertex.x());
}

int Walls::at(int row, int col) const
{
	return walls[row*width + col];
}

int Walls::at(const QPoint &vertex) const
{
	return at(vertex.y(), vertex.x());
}

int & Walls::at(int row, int col)
{
	return walls[row*width + col];
}

int & Walls::at(const QPoint &vertex)
{
	return at(vertex.y(), vertex.x());
}

const Walls::TextureIDs & Walls::internalTextureIdAt(int row, int col) const
{
	return texturemap[row*width + col];
}

const Walls::TextureIDs & Walls::internalTextureIdAt(const QPoint &vertex) const
{
	return internalTextureIdAt(vertex.y(), vertex.x());
}

Walls::TextureIDs & Walls::internalTextureIdAt(int row, int col)
{
	return texturemap[row*width + col];
}

Walls::TextureIDs & Walls::internalTextureIdAt(const QPoint &vertex)
{
	return internalTextureIdAt(vertex.y(), vertex.x());
}

const Walls::Heights & Walls::internalHeightsAt(int row, int col) const
{
	return heightmap[row*width + col];
}

const Walls::Heights & Walls::internalHeightsAt(const QPoint &vertex) const
{
	return internalHeightsAt(vertex.y(), vertex.x());
}

bool Walls::addWallBetweenVertices(const QPoint &a, const QPoint &b)
{
	const int distance = (b - a).manhattanLength();
	if (distance == 0 || distance > 1)
		return false;

	const bool containsA = contains(a);
	const bool containsB = contains(b);
	int &aV = at(a);
	int &bV = at(b);

	// printf("CONNECTING: (%i, %i) -> (%i, %i)\n", a.x(), a.y(), b.x(), b.y());

	if (a.x() < b.x() && a.x() >= 0 && a.x() < width && b.x() >= 0 && b.x() < width)
	{
		if (containsA) aV |= EastWall;
		if (containsB) bV |= WestWall;
	}
	else if (b.x() < a.x() && a.x() >= 0 && a.x() < width && b.x() >= 0 && b.x() < width)
	{
		if (containsB) bV |= EastWall;
		if (containsA) aV |= WestWall;
	}

	if (a.y() < b.y() && a.y() < height && b.y() >= 0 && b.y() < height)
	{
		if (containsA) aV |= SouthWall;
		if (containsB) bV |= NorthWall;
	}
	else if (b.y() < a.y() && a.y() < height && b.y() >= 0 && b.y() < height)
	{
		if (containsB) bV |= SouthWall;
		if (containsA) aV |= NorthWall;
	}

	// _RefreshTiles();
	return true;
}

void Walls::removeWallBetweenVertices(const QPoint &a, const QPoint &b)
{
	const int distance = (b - a).manhattanLength();
	if (distance == 0 || distance > 1)
		return;

	const bool containsA = contains(a);
	const bool containsB = contains(b);
	int &aV = at(a);
	int &bV = at(b);

	if (a.x() < b.x())
	{
		if (containsA) aV &= ~EastWall;
		if (containsB) bV &= ~WestWall;
	}
	else if (b.x() < a.x())
	{
		if (containsB) bV &= ~EastWall;
		if (containsA) aV &= ~WestWall;
	}

	if (a.y() < b.y())
	{
		if (containsA) aV &= ~SouthWall;
		if (containsB) bV &= ~NorthWall;
	}
	else if (b.y() < a.y())
	{
		if (containsB) bV &= ~SouthWall;
		if (containsA) aV &= ~NorthWall;
	}

	// _RefreshTiles();
}

void Walls::removeWallBetweenTiles(const QPoint &a, const QPoint &b)
{
	const int distance = (b - a).manhattanLength();
	if (distance == 0 || distance > 1)
		return;

	if (a.x() < b.x())
		removeWallBetweenVertices(a + QPoint(1, 0), a + QPoint(1, 1));
	else if (b.x() < a.x())
		removeWallBetweenVertices(b + QPoint(1, 0), b + QPoint(1, 1));
	else if (a.y() < b.y())
		removeWallBetweenVertices(a + QPoint(0, 1), a + QPoint(1, 1));
	else if (b.y() < a.y())
		removeWallBetweenVertices(b + QPoint(0, 1), b + QPoint(1, 1));
}
#include <QDebug>
void Walls::paintWall(const Orientation &p, const QString &filename)
{
	if (filename.size() == 0)
		return;
	/*if (!ids.contains(filename))
	{
		textures.insert(0, QPair<QString, unsigned int>("data/textures/" + filename, 0));
	}*/
	const int textureId = textures.addTexture(filename);
	// qDebug() << "Painting filename = " << filename << " textureId = " << textureId;
	switch (p.direction)
	{
		case Orientation::North:
		texturemap[p.tile.y()*width+p.tile.x()].east_southeast = textureId;
		texturemap[p.tile.y()*width+p.tile.x()].vertex = textureId;
		texturemap[p.tile.y()*width+p.tile.x()+1].vertex = textureId;
		break;

		case Orientation::South:
		texturemap[(p.tile.y()+1)*width+p.tile.x()].east_northeast = textureId;
		texturemap[(p.tile.y()+1)*width+p.tile.x()].vertex = textureId;
		texturemap[(p.tile.y()+1)*width+p.tile.x()+1].vertex = textureId;
		break;

		case Orientation::East:
		texturemap[p.tile.y()*width+p.tile.x()+1].south_southwest = textureId;
		texturemap[p.tile.y()*width+p.tile.x()+1].vertex = textureId;
		texturemap[(p.tile.y()+1)*width+p.tile.x()+1].vertex = textureId;
		break;

		case Orientation::West:
		texturemap[p.tile.y()*width+p.tile.x()].south_southeast = textureId;
		texturemap[p.tile.y()*width+p.tile.x()].vertex = textureId;
		texturemap[(p.tile.y()+1)*width+p.tile.x()].vertex = textureId;
		break;
	}
}

void Walls::setWallHeight(const Orientation &p, const int newHeight)
{
	switch (p.direction)
	{
		case Orientation::North:
		heightmap[p.tile.y()*width+p.tile.x()].east = newHeight;
		heightmap[p.tile.y()*width+p.tile.x()+1].west = newHeight;
		break;

		case Orientation::South:
		heightmap[(p.tile.y()+1)*width+p.tile.x()].east = newHeight;
		heightmap[(p.tile.y()+1)*width+p.tile.x()+1].west = newHeight;
		break;

		case Orientation::East:
		heightmap[p.tile.y()*width+p.tile.x()+1].south = newHeight;
		heightmap[(p.tile.y()+1)*width+p.tile.x()+1].north = newHeight;
		break;

		case Orientation::West:
		heightmap[p.tile.y()*width+p.tile.x()].south = newHeight;
		heightmap[(p.tile.y()+1)*width+p.tile.x()].north = newHeight;
		break;
	}
}

void Walls::setContext(QGLWidget *newContext)
{
	textures.setContext(newContext);
}
// #include <QDebug>
void Walls::draw(bool orthographicMode) const
{
	// qDebug() << "Started drawing...";
	for (WallTextures::InternalIdToIdMap::const_iterator it = textures.texturesAvailable().begin(); it != textures.texturesAvailable().end(); it++)
	{
		// qDebug() << "\tBinding" << it.key() << it.value();
		glBindTexture(GL_TEXTURE_2D, it.value());
		glBegin(GL_QUADS);
		int i = 0;
		for (int row = 0; row < height; row++)
		{
			for (int col = 0; col < width; col++, i++)
			{
				// the end caps
				if (texturemap[i].vertex == it.key())
				{
					switch (walls[i])
					{
						case NorthWall:
						glNormal3iv(northWallNormal);
						glTexCoord2i(0, 0); // upper north-west corner
						glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE, -GRID_SIZE*heightmap[i].north/10);
						glTexCoord2i(0, 1); // lower north-west corner
						glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE, 0);
						glTexCoord2i(1, 1); // lower north-east corner
						glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE, 0);
						glTexCoord2i(1, 0); // upper north-east corner
						glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE, -GRID_SIZE*heightmap[i].north/10);
						break;

						case SouthWall:
						glNormal3iv(southWallNormal);
						glTexCoord2i(1, 0); // upper south-west corner
						glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE, -GRID_SIZE*heightmap[i].south/10);
						glTexCoord2i(0, 0); // upper south-east corner
						glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE, -GRID_SIZE*heightmap[i].south/10);
						glTexCoord2i(0, 1); // lower south-east corner
						glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE, 0);
						glTexCoord2i(1, 1); // lower south-west corner
						glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE, 0);
						break;

						case WestWall:
						glNormal3iv(westWallNormal);
						glTexCoord2i(1, 0); // upper north-west corner
						glVertex3i(col*GRID_SIZE, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].west/10);
						glTexCoord2i(0, 0); // upper south-west corner
						glVertex3i(col*GRID_SIZE, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].west/10);
						glTexCoord2i(0, 1); // lower south-west corner
						glVertex3i(col*GRID_SIZE, row*GRID_SIZE+HALF_WALL_WIDTH, 0);
						glTexCoord2i(1, 1); // lower north-west corner
						glVertex3i(col*GRID_SIZE, row*GRID_SIZE-HALF_WALL_WIDTH, 0);
						break;

						case EastWall:
						glNormal3iv(eastWallNormal);
						glTexCoord2i(0, 0); // upper north-east corner
						glVertex3i(col*GRID_SIZE, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
						glTexCoord2i(0, 1); // lower north-east corner
						glVertex3i(col*GRID_SIZE, row*GRID_SIZE-HALF_WALL_WIDTH, 0);
						glTexCoord2i(1, 1); // lower south-east corner
						glVertex3i(col*GRID_SIZE, row*GRID_SIZE+HALF_WALL_WIDTH, 0);
						glTexCoord2i(1, 0); // upper south-east corner
						glVertex3i(col*GRID_SIZE, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
						break;
					}
				}
				if (walls[i] & EastWall)
				{
					float leftOffsetTop = (walls[i] & NorthWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i] == SouthEastCorner)
						leftOffsetTop = -HALF_WALL_WIDTH;
					float rightOffsetTop = (walls[i+1] & NorthWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i+1] == SouthWestCorner)
						rightOffsetTop = -HALF_WALL_WIDTH;

					float leftOffsetBottom = (walls[i] & SouthWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i] == NorthEastCorner)
						leftOffsetBottom = -HALF_WALL_WIDTH;
					float rightOffsetBottom = (walls[i+1] & SouthWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i+1] == NorthWestCorner)
						rightOffsetBottom = -HALF_WALL_WIDTH;

					if (texturemap[i].east_southeast == it.key())
					{
						glNormal3iv(northWallNormal);
						glTexCoord2i(0, 0); // upper north-west corner
						glVertex3i(col*GRID_SIZE+leftOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
						glTexCoord2i(0, 1); // lower north-west corner
						glVertex3i(col*GRID_SIZE+leftOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, 0);
						glTexCoord2i(1, 1); // lower north-east corner
						glVertex3i((col+1)*GRID_SIZE-rightOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, 0);
						glTexCoord2i(1, 0); // upper north-east corner
						glVertex3i((col+1)*GRID_SIZE-rightOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);

						// if (!orthographicMode)
						{
							glNormal3iv(floorNormal);
							glTexCoord2i(1, 1); // outer east corner
							glVertex3i((col+1)*GRID_SIZE-rightOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
							glTexCoord2i(1, 0); // inner east corner
							glVertex3i((col+1)*GRID_SIZE, row*GRID_SIZE, -GRID_SIZE*heightmap[i].east/10);
							glTexCoord2i(0, 0); // inner west corner
							glVertex3i(col*GRID_SIZE, row*GRID_SIZE, -GRID_SIZE*heightmap[i].east/10);
							glTexCoord2i(0, 1); // outer west corner
							glVertex3i(col*GRID_SIZE+leftOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
						}
					}

					if (texturemap[i].east_northeast == it.key())
					{
						glNormal3iv(southWallNormal);
						glTexCoord2i(1, 0); // upper south-west corner
						glVertex3i(col*GRID_SIZE+leftOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
						glTexCoord2i(0, 0); // upper south-east corner
						glVertex3i((col+1)*GRID_SIZE-rightOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
						glTexCoord2i(0, 1); // lower south-east corner
						glVertex3i((col+1)*GRID_SIZE-rightOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, 0);
						glTexCoord2i(1, 1); // lower south-west corner
						glVertex3i(col*GRID_SIZE+leftOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, 0);

						// if (!orthographicMode)
						{
							glTexCoord2i(0, 0); // outer west corner
							glVertex3i(col*GRID_SIZE+leftOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
							glTexCoord2i(0, 1); // inner west corner
							glVertex3i(col*GRID_SIZE, row*GRID_SIZE, -GRID_SIZE*heightmap[i].east/10);
							glTexCoord2i(1, 1); // inner east corner
							glVertex3i((col+1)*GRID_SIZE, row*GRID_SIZE, -GRID_SIZE*heightmap[i].east/10);
							glTexCoord2i(1, 0); // outer east corner
							glVertex3i((col+1)*GRID_SIZE-rightOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE*heightmap[i].east/10);
						}
					}
				}
				if (walls[i] & SouthWall)
				{
					float topOffsetLeft = (walls[i] & WestWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i] == SouthEastCorner)
						topOffsetLeft = -HALF_WALL_WIDTH;
					float bottomOffsetLeft = (walls[i+width] & WestWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i+width] == NorthEastCorner)
						bottomOffsetLeft = -HALF_WALL_WIDTH;

					float topOffsetRight = (walls[i] & EastWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i] == SouthWestCorner)
						topOffsetRight = -HALF_WALL_WIDTH;
					float bottomOffsetRight = (walls[i+width] & EastWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i+width] == NorthWestCorner)
						bottomOffsetRight = -HALF_WALL_WIDTH;

					if (texturemap[i].south_southeast == it.key())
					{
						glNormal3iv(westWallNormal);
						glTexCoord2i(1, 0); // upper north-west corner
						glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetRight, -GRID_SIZE*heightmap[i].south/10);
						glTexCoord2i(0, 0); // upper south-west corner
						glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetRight, -GRID_SIZE*heightmap[i].south/10);
						glTexCoord2i(0, 1); // lower south-west corner
						glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetRight, 0);
						glTexCoord2i(1, 1); // lower north-west corner
						glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetRight, 0);

						// if (!orthographicMode)
						{
							glTexCoord2i(0, 0); // inner north corner
							glVertex3i(col*GRID_SIZE, row*GRID_SIZE, -GRID_SIZE*heightmap[i].south/10);
							glTexCoord2i(0, 1); // inner south corner
							glVertex3i(col*GRID_SIZE, (row+1)*GRID_SIZE, -GRID_SIZE*heightmap[i].south/10);
							glTexCoord2i(1, 1); // outer south corner
							glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetRight, -GRID_SIZE*heightmap[i].south/10);
							glTexCoord2i(1, 0); // outer north corner
							glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetRight, -GRID_SIZE*heightmap[i].south/10);
						}
					}

					if (texturemap[i].south_southwest == it.key())
					{
						glNormal3iv(eastWallNormal);
						glTexCoord2i(0, 0); // upper north-east corner
						glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetLeft, -GRID_SIZE*heightmap[i].south/10);
						glTexCoord2i(0, 1); // lower north-east corner
						glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetLeft, 0);
						glTexCoord2i(1, 1); // lower south-east corner
						glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetLeft, 0);
						glTexCoord2i(1, 0); // upper south-east corner
						glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetLeft, -GRID_SIZE*heightmap[i].south/10);

						// if (!orthographicMode)
						{
							glNormal3iv(eastWallNormal);
							glTexCoord2i(1, 1); // inner south corner
							glVertex3i(col*GRID_SIZE, (row+1)*GRID_SIZE, -GRID_SIZE*heightmap[i].south/10);
							glTexCoord2i(0, 1); // inner north corner
							glVertex3i(col*GRID_SIZE, row*GRID_SIZE, -GRID_SIZE*heightmap[i].south/10);
							glTexCoord2i(0, 0); // outer north corner
							glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetLeft, -GRID_SIZE*heightmap[i].south/10);
							glTexCoord2i(0, 1); // outer south corner
							glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetLeft, -GRID_SIZE*heightmap[i].south/10);
						}
					}
				}
			}
		}
		glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// qDebug() << "Finished drawing";
}

void Walls::drawTops() const
{
	glColor3f(0.0, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_QUADS);
	{
		int i = 0;
		for (int row = 0; row < height; row++)
		{
			for (int col = 0; col < width; col++, i++)
			{
				const float centerX = static_cast<float>(col*GRID_SIZE);
				const float centerY = static_cast<float>(row*GRID_SIZE);

				// TODO bevelling
				if (walls[i]/* == SouthEastCorner*/)
				{
					glVertex3i(centerX-HALF_WALL_WIDTH, centerY-HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX-HALF_WALL_WIDTH, centerY+HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX+HALF_WALL_WIDTH, centerY+HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX+HALF_WALL_WIDTH, centerY-HALF_WALL_WIDTH, -GRID_SIZE);
				}
				if (walls[i] & EastWall)
				{
					glVertex3i(centerX+HALF_WALL_WIDTH, centerY-HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX+HALF_WALL_WIDTH, centerY+HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX+GRID_SIZE-HALF_WALL_WIDTH, centerY+HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX+GRID_SIZE-HALF_WALL_WIDTH, centerY-HALF_WALL_WIDTH, -GRID_SIZE);
				}
				if (walls[i] & SouthWall)
				{
					glVertex3i(centerX-HALF_WALL_WIDTH, centerY+HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX-HALF_WALL_WIDTH, centerY+GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX+HALF_WALL_WIDTH, centerY+GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE);
					glVertex3i(centerX+HALF_WALL_WIDTH, centerY+HALF_WALL_WIDTH, -GRID_SIZE);
				}
			}
		}
	}
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
}
