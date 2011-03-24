#include "Orientation.h"
#include "defines.h"

#include <QGLWidget>

bool Orientation::operator<(const Orientation &other) const
{
	if (tile != other.tile)
	{
		if (tile.x() != other.tile.x())
			return tile.x() < other.tile.x();
		else
			return tile.y() < other.tile.y();
	}
	else
		return direction < other.direction;
}

bool Orientation::operator==(const Orientation &other) const
{
	return tile == other.tile && direction == other.direction;
}

bool Orientation::operator!=(const Orientation &other) const
{
	return tile != other.tile || direction != other.direction;
}

QString Orientation::directionToString() const
{
	switch (direction)
	{
		case Orientation::North: return "north";
		case Orientation::East: return "east";
		case Orientation::South: return "south";
		case Orientation::West: return "west";
		
		default:
		return "north";
	}
}

Orientation::Direction Orientation::directionFromString(const QString &name)
{
	if (name == "north") return Orientation::North;
	if (name == "south") return Orientation::South;
	if (name == "east") return Orientation::East;
	if (name == "west") return Orientation::West;
	
	return Orientation::North;
}

void Orientation::draw() const
{
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);
	switch (direction)
	{
		/*glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH, -GRID_SIZE);*/
		
		case Orientation::North:
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH + PICTURE_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH + PICTURE_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH, -GRID_SIZE);
		break;
		
		case Orientation::East:
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH - PICTURE_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH - PICTURE_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH, -GRID_SIZE);
		break;
		
		case Orientation::South:
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH - PICTURE_WIDTH, -GRID_SIZE);
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i((tile.x() + 1)*GRID_SIZE - HALF_WALL_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH - PICTURE_WIDTH, -GRID_SIZE);
		break;
		
		case Orientation::West:
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH + PICTURE_WIDTH, (tile.y() + 1)*GRID_SIZE - HALF_WALL_WIDTH, -GRID_SIZE);
		glVertex3i(tile.x()*GRID_SIZE + HALF_WALL_WIDTH + PICTURE_WIDTH, tile.y()*GRID_SIZE + HALF_WALL_WIDTH, -GRID_SIZE);
		break;
	}
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
}
