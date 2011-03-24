#include "Tiles.h"
#include "Walls.h"
#include "defines.h"

#include "GLee.h"

#include <QGLWidget>
#include <QStack>

Tiles::Tiles() : width(0), height(0), context(NULL), floorTexture(0), ceilingTexture(0)
{
}

Tiles::~Tiles()
{
}

bool Tiles::operator==(const Tiles &other) const
{
	return tiles == other.tiles;
}

bool Tiles::operator!=(const Tiles &other) const
{
	return tiles != other.tiles;
}

void Tiles::resize(int w, int h)
{
	width = w;
	height = h;
	tiles.resize(width*height);
}

void Tiles::clear()
{
	tiles.fill(false);
}

bool Tiles::contains(int row, int col) const
{
	return row >= 0 && col >= 0 && row < height && col < width;
}

bool Tiles::contains(const QPoint &tile) const
{
	return contains(tile.y(), tile.x());
}

bool Tiles::at(int row, int col) const
{
	return tiles[row*width + col];
}

bool Tiles::at(const QPoint &tile) const
{
	return at(tile.y(), tile.x());
}

bool & Tiles::at(int row, int col)
{
	return tiles[row*width + col];
}

bool & Tiles::at(const QPoint &tile)
{
	return at(tile.y(), tile.x());
}

void Tiles::floodFill(int row, int col, const Walls &walls)
{
	floodFill(QPoint(col, row), walls);
}

void Tiles::floodFill(const QPoint &tile, const Walls &walls)
{
	QStack<QPoint> toFill;
	toFill.push(tile);
	while (!toFill.empty())
	{
		const QPoint p = toFill.pop();
		if (!contains(p))
			continue;
		if (at(p))
			continue;
		
		at(p) = true;
		
		if (!(walls.at(p) & Walls::EastWall))
			toFill.push(p + QPoint(0, -1));
		if (!(walls.at(p + QPoint(0, 1)) & Walls::EastWall))
			toFill.push(p + QPoint(0, 1));
		if (!(walls.at(p) & Walls::SouthWall))
			toFill.push(p + QPoint(-1, 0));
		if (!(walls.at(p + QPoint(1, 0)) & Walls::SouthWall))
			toFill.push(p + QPoint(1, 0));
	}
}

void Tiles::setContext(QGLWidget *newContext)
{
	if (context)
	{
		context->deleteTexture(floorTexture);
		context->deleteTexture(ceilingTexture);
	}
	context = newContext;
	if (context)
	{
		floorTexture = context->bindTexture(QPixmap("data/images/floor.jpg"));
		ceilingTexture = context->bindTexture(QPixmap("data/images/ceiling.jpg"));
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
		{
			float maximumAnisotropy = 1.1;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(4.0f, maximumAnisotropy));
		}
		glBindTexture(GL_TEXTURE_2D, ceilingTexture);
		if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
		{
			float maximumAnisotropy = 1.1;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(4.0f, maximumAnisotropy));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		floorTexture = 0;
		ceilingTexture = 0;
	}
}

void Tiles::drawGrid() const
{
	// glColor3f(0.85, 0.85, 0.85);
	// glColor3f(0.0, 0.0, 0.0);
	// glColor3f(0.5, 0.5, 0.5);
	glColor3f(1.0, 1.0, 1.0);
	// if (usingLines)
	{
		glLineWidth(1.0);
		glBegin(GL_LINES);
		for (int row = 0; row <= height; row++)
		{
			glVertex3f(0, row*GRID_SIZE, -0.02);
			glVertex3f(width*GRID_SIZE, row*GRID_SIZE, -0.02);
		}
		for (int col = 0; col <= width; col++)
		{
			glVertex3f(col*GRID_SIZE, 0, -0.02);
			glVertex3f(col*GRID_SIZE, height*GRID_SIZE, -0.02);
		}
		glEnd();
	}
	// else
	{
		glBegin(GL_QUADS);
		glNormal3iv(floorNormal);
		for (int row = 0; row <= height; row++)
		{
			glVertex3f(                - LINE_WIDTH/2, row*GRID_SIZE - LINE_WIDTH/2, -0.01); // north west corner of west-east "line"
			glVertex3f(                - LINE_WIDTH/2, row*GRID_SIZE + LINE_WIDTH/2, -0.01); // south west corner of west-east "line"
			glVertex3f(width*GRID_SIZE + LINE_WIDTH/2, row*GRID_SIZE + LINE_WIDTH/2, -0.01); // south east corner of west-east "line"
			glVertex3f(width*GRID_SIZE + LINE_WIDTH/2, row*GRID_SIZE - LINE_WIDTH/2, -0.01); // north east corner of west-east "line"
		}
		for (int col = 0; col <= width; col++)
		{
			glVertex3f(col*GRID_SIZE - LINE_WIDTH/2,                  - LINE_WIDTH/2, -0.01); // north west corner of west-east "line"
			glVertex3f(col*GRID_SIZE - LINE_WIDTH/2, height*GRID_SIZE + LINE_WIDTH/2, -0.01); // south west corner of west-east "line"
			glVertex3f(col*GRID_SIZE + LINE_WIDTH/2, height*GRID_SIZE + LINE_WIDTH/2, -0.01); // south east corner of west-east "line"
			glVertex3f(col*GRID_SIZE + LINE_WIDTH/2,                  - LINE_WIDTH/2, -0.01); // north east corner of west-east "line"
		}
		glEnd();
	}
	
	glColor3f(1.0, 1.0, 1.0);
}

void Tiles::drawFloor() const
{
	glBindTexture(GL_TEXTURE_2D, floorTexture);
	glBegin(GL_QUADS);
	glNormal3iv(floorNormal);
	{
		int i = 0;
		for (int row = 0; row < height; row++)
		{
			for (int col = 0; col < width; col++, i++)
			{
				if (tiles[i])
				{
					glTexCoord2i(0, 0); // lower north-west corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE, 0);

					glTexCoord2i(0, 1); // lower south-west corner
					glVertex3i(col*GRID_SIZE, (row+1)*GRID_SIZE, 0);

					glTexCoord2i(1, 1); // lower south-east corner
					glVertex3i((col+1)*GRID_SIZE, (row+1)*GRID_SIZE, 0);

					glTexCoord2i(1, 0); // lower north-east corner
					glVertex3i((col+1)*GRID_SIZE, row*GRID_SIZE, 0);
				}
			}
		}
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Tiles::drawCeiling() const
{
	glBindTexture(GL_TEXTURE_2D, ceilingTexture);
	glBegin(GL_QUADS);
	glNormal3iv(ceilingNormal);
	{
		int i = 0;
		for (int row = 0; row < height; row++)
		{
			for (int col = 0; col < width; col++, i++)
			{
				if (tiles[i])
				{
					glTexCoord2i(0, 0); // north-east corner
					glVertex3i((col+1)*GRID_SIZE, row*GRID_SIZE, -GRID_SIZE);

					glTexCoord2i(0, 1); // south-east corner
					glVertex3i((col+1)*GRID_SIZE, (row+1)*GRID_SIZE, -GRID_SIZE);

					glTexCoord2i(1, 1); // south-west corner
					glVertex3i(col*GRID_SIZE, (row+1)*GRID_SIZE, -GRID_SIZE);

					glTexCoord2i(1, 0); // north-west corner
					glVertex3i(col*GRID_SIZE, row*GRID_SIZE, -GRID_SIZE);
				}
			}
		}
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}
