#include "Walls.h"
#include "defines.h"

#include <QFile>
#include <QString>
#include <QGLWidget>

#include "Glee.h"

Walls::Walls() : width(0), height(0), context(NULL), wallTexture(0)
{
}

Walls::~Walls()
{
}

bool Walls::operator==(const Walls &other) const
{
	return walls == other.walls;
}

bool Walls::operator!=(const Walls &other) const
{
	return walls != other.walls;
}

void Walls::resize(int w, int h)
{
	width = w + 1;
	height = h + 1;
	walls.resize(width*height);
}

void Walls::clear()
{
	walls.fill(NoWalls);
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
		for (int col = 0; col < width; col++)
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
		for (int col = 1; col < width; col++)
		{
			if (at(row-1, col) & Walls::SouthWall)
				at(row, col) |= Walls::NorthWall;
			if (at(row, col-1) & Walls::EastWall)
				at(row, col) |= Walls::WestWall;
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

void Walls::setContext(QGLWidget *newContext)
{
	if (context)
		context->deleteTexture(wallTexture);
	context = newContext;
	if (context)
	{
		wallTexture = context->bindTexture(QPixmap("data/images/wall.jpg"));
		if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
		{
			float maximumAnisotropy = 1.1;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);
		}
	}
	else
		wallTexture = 0;
}

void Walls::draw() const
{
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glBegin(GL_QUADS);
	{
		int i = 0;
		for (int row = 0; row < height; row++)
		{
			for (int col = 0; col < width; col++, i++)
			{
				// the end caps
				switch (walls[i])
				{
					case NorthWall:
					glNormal3iv(northWallNormal);
					glTexCoord2i(0, 0); // upper north-west corner
					glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE, -GRID_SIZE);
					glTexCoord2i(0, 1); // lower north-west corner
					glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE, 0);
					glTexCoord2i(1, 1); // lower north-east corner
					glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE, 0);
					glTexCoord2i(1, 0); // upper north-east corner
					glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE, -GRID_SIZE);
					break;
					
					case SouthWall:
					glNormal3iv(southWallNormal);
					glTexCoord2i(1, 0); // upper south-west corner
					glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE, -GRID_SIZE);
					glTexCoord2i(0, 0); // upper south-east corner
					glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE, -GRID_SIZE);
					glTexCoord2i(0, 1); // lower south-east corner
					glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE, 0);
					glTexCoord2i(1, 1); // lower south-west corner
					glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE, 0);
					break;
					
					case WestWall:
					glNormal3iv(westWallNormal);
					glTexCoord2i(1, 0); // upper north-west corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE);
					glTexCoord2i(0, 0); // upper south-west corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE);
					glTexCoord2i(0, 1); // lower south-west corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE+HALF_WALL_WIDTH, 0);
					glTexCoord2i(1, 1); // lower north-west corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE-HALF_WALL_WIDTH, 0);
					break;
					
					case EastWall:
					glNormal3iv(eastWallNormal);
					glTexCoord2i(0, 0); // upper north-east corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE);
					glTexCoord2i(0, 1); // lower north-east corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE-HALF_WALL_WIDTH, 0);
					glTexCoord2i(1, 1); // lower south-east corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE+HALF_WALL_WIDTH, 0);
					glTexCoord2i(1, 0); // upper south-east corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE);
					break;
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
					
					glNormal3iv(northWallNormal);
					glTexCoord2i(0, 0); // upper north-west corner
					glVertex3i(col*GRID_SIZE+leftOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE);
					glTexCoord2i(0, 1); // lower north-west corner
					glVertex3i(col*GRID_SIZE+leftOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, 0);
					glTexCoord2i(1, 1); // lower north-east corner
					glVertex3i((col+1)*GRID_SIZE-rightOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, 0);
					glTexCoord2i(1, 0); // upper north-east corner
					glVertex3i((col+1)*GRID_SIZE-rightOffsetBottom, row*GRID_SIZE+HALF_WALL_WIDTH, -GRID_SIZE);
					
					glNormal3iv(southWallNormal);
					glTexCoord2i(1, 0); // upper south-west corner
					glVertex3i(col*GRID_SIZE+leftOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE);
					glTexCoord2i(0, 0); // upper south-east corner
					glVertex3i((col+1)*GRID_SIZE-rightOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, -GRID_SIZE);
					glTexCoord2i(0, 1); // lower south-east corner
					glVertex3i((col+1)*GRID_SIZE-rightOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, 0);
					glTexCoord2i(1, 1); // lower south-west corner
					glVertex3i(col*GRID_SIZE+leftOffsetTop, row*GRID_SIZE-HALF_WALL_WIDTH, 0);
				}
				if (walls[i] & SouthWall)
				{
					float topOffsetLeft = (walls[i] & WestWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i] == SouthEastCorner)
						topOffsetLeft = -HALF_WALL_WIDTH;
					float bottomOffsetLeft = (walls[i+(width+1)] & WestWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i+width] == NorthEastCorner)
						bottomOffsetLeft = -HALF_WALL_WIDTH;
					
					float topOffsetRight = (walls[i] & EastWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i] == SouthWestCorner)
						topOffsetRight = -HALF_WALL_WIDTH;
					float bottomOffsetRight = (walls[i+(width+1)] & EastWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i+width] == NorthWestCorner)
						bottomOffsetRight = -HALF_WALL_WIDTH;
					
					glNormal3iv(westWallNormal);
					glTexCoord2i(1, 0); // upper north-west corner
					glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetRight, -GRID_SIZE);
					glTexCoord2i(0, 0); // upper south-west corner
					glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetRight, -GRID_SIZE);
					glTexCoord2i(0, 1); // lower south-west corner
					glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetRight, 0);
					glTexCoord2i(1, 1); // lower north-west corner
					glVertex3i(col*GRID_SIZE+HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetRight, 0);
					
					glNormal3iv(eastWallNormal);
					glTexCoord2i(0, 0); // upper north-east corner
					glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetLeft, -GRID_SIZE);
					glTexCoord2i(0, 1); // lower north-east corner
					glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, row*GRID_SIZE+topOffsetLeft, 0);
					glTexCoord2i(1, 1); // lower south-east corner
					glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetLeft, 0);
					glTexCoord2i(1, 0); // upper south-east corner
					glVertex3i(col*GRID_SIZE-HALF_WALL_WIDTH, (row+1)*GRID_SIZE-bottomOffsetLeft, -GRID_SIZE);
				}
			}
		}
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
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
