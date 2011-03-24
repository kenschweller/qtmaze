#include "Maze.h"
#include "defines.h"

#include <QGLWidget>
#include <QFile>
#include <QTextStream>

#include <cstdlib>

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

uint qHash(const QPoint &p)
{
	return (p.y() << 16) + p.x();
}

Maze::Maze() : context(NULL), floorTexture(0), ceilingTexture(0), wallTexture(0)
{
	reset(16, 16);
}

Maze & Maze::operator=(const Maze &other)
{
	width = other.width;
	height = other.height;
	walls = other.walls;
	tiles = other.tiles;
	pictures = other.pictures;
	startingOrientation = other.startingOrientation;
	goalTiles = other.goalTiles;
	
	return *this;
	// pictureTextures = other.pictureTextures;
}

bool Maze::operator==(const Maze &other) const
{
	return
	(
		width == other.width &&
		height == other.height &&
		walls == other.walls &&
		tiles == other.tiles &&
		pictures == other.pictures &&
		startingOrientation == other.startingOrientation &&
		goalTiles == other.goalTiles
	);
}

void Maze::reset(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	tiles.resize(width*height);
	walls.resize((width+1)*(height + 1));
	tiles.fill(0);
	walls.fill(0);
	for (int i = 0; i < tiles.size(); i++)
		tiles[i] = false;
	startingOrientation = Orientation();
	goalTiles.clear();
	goalTiles.insert(QPoint(0, 0));
	pictures.clear();
	_RefreshTiles();
}

bool Maze::load(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	
	int newWidth = 0;
	int newHeight = 0;
	QVector<bool> newTiles;
	QVector<int> newWalls;
	GoalSet newGoalTiles;
	Pictures newPictures;
	newPictures.setContext(context);
	Orientation newStartingOrientation;
	
	while (!file.atEnd())
	{
		const QString trimmedLine = file.readLine().trimmed();
		const QStringList words = trimmedLine.simplified().split(" ");
		if (words.size() == 0)
			continue;
		
		const QString sectionName = words[0].toLower();
		
		fflush(stdout);
		if (sectionName == "width")
		{
			if (words.size() < 2)
				return false;
			newWidth = words[1].toInt();
		}
		else if (sectionName == "height")
		{
			if (words.size() < 2)
				return false;
			newHeight = words[1].toInt();
		}
		else if (sectionName == "tiles")
		{
			for (int row = 0; row < newHeight; row++)
			{
				if (!file.canReadLine())
					return false;
				const QString line = file.readLine().trimmed();
				if (line.size() < newWidth)
					return false;
				for (int col = 0; col < newWidth; col++)
					newTiles[row*newWidth+col] = (line[col] == 'X');
			}
		}
		else if (sectionName == "walls")
		{
			for (int row = 0; row < newHeight; row++)
			{
				if (!file.canReadLine())
					return false;
				const QString line = file.readLine().trimmed();
				if (line.size() < newWidth)
					return false;
				for (int col = 0; col < newWidth; col++)
				{
					const QChar c = line[col];
					if (c == '+' || c == '-')
						newWalls[row*(newWidth+1)+col] |= EastWall;
					if (c == '+' || c == '|')
						newWalls[row*(newWidth+1)+col] |= SouthWall;
				}
			}
		}
		else if (sectionName == "start")
		{
			if (words.size() < 7)
				return false;
			if (words[1].toLower() != "row" || words[3].toLower() != "col" || words[5] != "side")
				return false;
			bool rowOk = false, colOk = false;
			const QPoint startTile(words[4].toInt(&colOk), words[2].toInt(&rowOk));
			if (!rowOk || !colOk)
				return false;
			newStartingOrientation = Orientation(startTile, Orientation::directionFromString(words[6]));
		}
		else if (sectionName == "goal")
		{
			if (words.size() < 5)
				return false;	
			if (words[1].toLower() != "row" || words[3].toLower() != "col")
				return false;
			bool rowOk = false, colOk = false;
			const QPoint goalTile(words[4].toInt(&colOk), words[2].toInt(&rowOk));
			if (!rowOk || !colOk)
				return false;
			// if (goalTile.x() < 0 || goalTile.y() < 0 || goalTile.x() >= newWidth || goalTile.y() >= newHeight)
			newGoalTiles.insert(goalTile);
			
		}
		else if (sectionName == "picture")
		{
			if (words.size() < 9)
				return false;
			if (words[1].toLower() != "row" || words[3].toLower() != "col" || words[5] != "side")
				return false;
			bool rowOk = false, colOk = false;
			const QPoint pictureTile(words[4].toInt(&colOk), words[2].toInt(&rowOk));
			if (!rowOk || !colOk)
				return false;
			const Orientation pictureOrientation(pictureTile, Orientation::directionFromString(words[6]));
			const int filenameStart = words[8].indexOf('"') + 1;
			const int filenameEnd = words[8].lastIndexOf('"');
			newPictures.add(pictureOrientation, words[8].mid(filenameStart, filenameEnd - filenameStart));
		}
		
		// use the new dimensions
		if (newWidth > 0 && newHeight > 0 && newTiles.size() != newWidth*newHeight)
		{
			newTiles.resize(0);
			newWalls.resize(0);
			newTiles.resize(newWidth * newHeight);
			newWalls.resize((newWidth + 1) * (newHeight + 1));
		}
	}
	
	if (newWidth < 1 || newHeight < 1)
		return false;
	
	width = newWidth;
	height = newHeight;
	tiles = newTiles;
	walls = newWalls;
	goalTiles = newGoalTiles;
	if (goalTiles.empty())
		goalTiles.insert(QPoint(0, 0));
	startingOrientation = newStartingOrientation;
	pictures = newPictures;
	
	for (int row = 1; row <= newHeight; row++)
	{
		for (int col = 1; col <= newWidth; col++)
		{
			if (vertexAt(row-1, col) & SouthWall)
				vertexAt(row, col) |= NorthWall;
			if (vertexAt(row, col-1) & EastWall)
				vertexAt(row, col) |= WestWall;
		}
	}
	
	_RefreshTiles();
	
	return true;
}

bool Maze::save(const QString &filename) const
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;
	
	QTextStream out(&file);
	out << "width " << width << "\n";
	out << "height " << height << "\n";
	
	out << "walls\n";
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if ((vertexAt(row, col) & SouthEastCorner) == SouthEastCorner)
				out << "+";
			else if (vertexAt(row, col) & EastWall)
				out << "-";
			else if (vertexAt(row, col) & SouthWall)
				out << "|";
			else
				out << ".";
		}
		out << "\n";
	}
	
	out << "start row " << startingOrientation.tile.y() << " col " << startingOrientation.tile.x() << " side " << startingOrientation.directionToString() << "\n";
	
	for (GoalSet::const_iterator it = goalTiles.begin(); it != goalTiles.end(); ++it)
	{
		out << "goal row " << it->y() << " col " << it->x() << "\n";
	}
	
	out << pictures;
	
	return true;
}

// #include <QMessageBox>
void Maze::setContext(QGLWidget *c)
{
	if (context)
	{
		context->deleteTexture(floorTexture);
		floorTexture = 0;

		context->deleteTexture(ceilingTexture);
		ceilingTexture = 0;

		context->deleteTexture(wallTexture);
		wallTexture = 0;
	}
	context = c;
	if (context)
	{
		floorTexture = context->bindTexture(QPixmap("data/images/floor.jpg"));
		ceilingTexture = context->bindTexture(QPixmap("data/images/ceiling.jpg"));
		wallTexture = context->bindTexture(QPixmap("data/images/wall.jpg"));
	}
	
	pictures.setContext(c);
}

Camera Maze::getStartingCamera()
{
	Camera result;
	switch (startingOrientation.direction)
	{
		case Orientation::North: result.view = QVector3D( 0.0,  1.0,  0.0); break;
		case Orientation::South: result.view = QVector3D( 0.0, -1.0,  0.0); break;
		case Orientation::East:  result.view = QVector3D(-1.0,  0.0,  0.0); break;
		case Orientation::West:  result.view = QVector3D( 1.0,  0.0,  0.0); break;
	}
	result.up   = QVector3D(0.0,  0.0, -1.0);
	result.position = QVector3D(startingOrientation.tile.x()*GRID_SIZE + GRID_SIZE/2.0, startingOrientation.tile.y()*GRID_SIZE + GRID_SIZE/2.0, -GRID_SIZE/2.0);
	return result;
}

float Maze::getWidth() const
{
	return static_cast<float>(width) * GRID_SIZE;
}

float Maze::getHeight() const
{
	return static_cast<float>(height) * GRID_SIZE;
}

Orientation Maze::getStartingOrientation() const
{
	return startingOrientation;
}

bool Maze::containsTile(const QPoint &tile) const
{
	return tile.x() >= 0 && tile.y() >= 0 && tile.x() < width && tile.y() < height;
}

bool Maze::containsVertex(const QPoint &vertex) const
{
	return vertex.x() >= 0 && vertex.y() >= 0 && vertex.x() <= width && vertex.y() <= height;
}

bool Maze::mapPointInGoalRadius(const QPoint &point) const
{
	const QPoint tile = getNearestTile(point);
	if (!containsTile(tile))
		return false;
	if (!goalTiles.contains(tile))
		return false;
	
	const QLineF line(tile.x()*GRID_SIZE+GRID_SIZE/2.0, tile.y()*GRID_SIZE+GRID_SIZE/2.0, point.x(), point.y());
	
	return line.length() <= 50.0;
}

QPoint Maze::getNearestVertex(const QPoint &point) const
{
	QPoint result = point/GRID_SIZE;
	return result;
}

QPoint Maze::getNearestTile(const QPoint &point) const
{
	QPoint result = (point - QPoint(GRID_SIZE/2, GRID_SIZE/2))/GRID_SIZE;
	return result;
}
#include <QLineF>
Orientation Maze::getNearestOrientation(const QPoint &point) const
{
	Orientation orientation;
	orientation.tile = getNearestTile(point);
	
	QLineF line(orientation.tile.x() * GRID_SIZE + GRID_SIZE/2, orientation.tile.y() * GRID_SIZE + GRID_SIZE/2, point.x(), point.y());
	qreal angle = (line.angle() - 45.0)/90.0;
	
	if (angle >= 0.0 && angle < 1.0)
		orientation.direction = Orientation::North;
	else if (angle >= 1.0 && angle < 2.0)
		orientation.direction = Orientation::West;
	else if (angle >= 2.0 && angle < 3.0)
		orientation.direction = Orientation::South;
	else
		orientation.direction = Orientation::East;
	
	return orientation;
}

Pictures::Picture Maze::getPicture(const Orientation &orientation) const
{
	return pictures.at(orientation);
}

void Maze::addPicture(const Orientation &orientation, const QString &filename)
{
	if (!containsTile(orientation.tile))
		return;
	removePicture(orientation);
	
	pictures.add(orientation, filename);
	/*if (!pictureTextures.contains(filename))
		pictureTextures.insert(filename, (context == NULL) ? 0 : context->bindTexture(QPixmap("data/landmarks/" + filename)));*/
}

void Maze::removePicture(const Orientation &orientation)
{
	pictures.remove(orientation);
}

void Maze::addGoal(const QPoint &p)
{
	if (containsTile(p))
		goalTiles.insert(p);
}

void Maze::removeGoal(const QPoint &p)
{
	goalTiles.remove(p);
}

void Maze::setStartingOrientation(const Orientation &orientation)
{
	if (containsTile(orientation.tile))
	{
		startingOrientation = orientation;
		_RefreshTiles();
	}
}

// #include <cstdio>
bool Maze::connectVertices(const QPoint &a, const QPoint &b)
{
	const int distance = (b - a).manhattanLength();
	if (distance == 0 || distance > 1)
		return false;

	const bool containsA = containsVertex(a);	
	const bool containsB = containsVertex(b);	
	int &aV = vertexAt(a);
	int &bV = vertexAt(b);

	// printf("CONNECTING: (%i, %i) -> (%i, %i)\n", a.x(), a.y(), b.x(), b.y());
	
	if (a.x() < b.x() && a.x() >= 0 && a.x() <= width && b.x() >= 0 && b.x() <= width)
	{
		if (containsA) aV |= EastWall;
		if (containsB) bV |= WestWall;
	}
	else if (b.x() < a.x() && a.x() >= 0 && a.x() <= width && b.x() >= 0 && b.x() <= width)
	{
		if (containsB) bV |= EastWall;
		if (containsA) aV |= WestWall;
	}

	if (a.y() < b.y() && a.y() <= height && b.y() >= 0 && b.y() <= height)
	{
		if (containsA) aV |= SouthWall;
		if (containsB) bV |= NorthWall;
	}
	else if (b.y() < a.y() && a.y() <= height && b.y() >= 0 && b.y() <= height)
	{
		if (containsB) bV |= SouthWall;
		if (containsA) aV |= NorthWall;
	}

	_RefreshTiles();
	return true;
}

void Maze::disconnectVertices(const QPoint &a, const QPoint &b)
{
	const int distance = (b - a).manhattanLength();
	if (distance == 0 || distance > 1)
		return;

	const bool containsA = containsVertex(a);
	const bool containsB = containsVertex(b);
	int &aV = vertexAt(a);
	int &bV = vertexAt(b);

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
	
	_RefreshTiles();
}

void Maze::removeWall(const QPoint &a, const QPoint &b)
{
	const int distance = (b - a).manhattanLength();
	if (distance == 0 || distance > 1)
		return;

	if (a.x() < b.x())
		disconnectVertices(a + QPoint(1, 0), a + QPoint(1, 1));
	else if (b.x() < a.x())
		disconnectVertices(b + QPoint(1, 0), b + QPoint(1, 1));
	else if (a.y() < b.y())
		disconnectVertices(a + QPoint(0, 1), a + QPoint(1, 1));
	else if (b.y() < a.y())
		disconnectVertices(b + QPoint(0, 1), b + QPoint(1, 1));
}

void Maze::startDroppingImage(const QString &filename)
{
	dropping.dragging = Dropping::DRAGGING_IMAGE;
	dropping.filename = filename;
}

void Maze::moveDroppingImage(const QPoint &p)
{
	dropping.orientation = getNearestOrientation(p);
}

void Maze::endDroppingImage(bool dropped)
{
	dropping.dragging = Dropping::DRAGGING_NONE;
	if (dropped)
		addPicture(dropping.orientation, dropping.filename);
}

void Maze::startDroppingGoal()
{
	dropping.dragging = Dropping::DRAGGING_GOAL;
}

void Maze::moveDroppingGoal(const QPoint &p)
{
	dropping.orientation = getNearestOrientation(p);
}

void Maze::endDroppingGoal(bool dropped)
{
	dropping.dragging = Dropping::DRAGGING_NONE;
	if (dropped)
		addGoal(dropping.orientation.tile);
}

#include <cmath>
QPointF Maze::addDisplacement(const QPointF &position, QPointF displacement) const
{
	const QPoint tile = getNearestTile(position.toPoint());
	bool northWall = false, eastWall = false, southWall = false, westWall = false;
	bool northWestCorner = false, northEastCorner = false, southEastCorner = false, southWestCorner = false;
	if (containsTile(tile))
	{
		northWall = vertexAt(tile)                & EastWall;
		eastWall  = vertexAt(tile + QPoint(1, 0)) & SouthWall;
		southWall = vertexAt(tile + QPoint(0, 1)) & EastWall;
		westWall  = vertexAt(tile)                & SouthWall;
		
		northWestCorner = vertexAt(tile)                & NorthWestCorner;
		northEastCorner = vertexAt(tile + QPoint(1, 0)) & NorthEastCorner;
		southEastCorner = vertexAt(tile + QPoint(1, 1)) & SouthEastCorner;
		southWestCorner = vertexAt(tile + QPoint(0, 1)) & SouthWestCorner;
	}
	
	static const float PLAYER_RADIUS = 100.0;
	const float northWallDistance = fmod(position.y(), GRID_SIZE);
	const float westWallDistance  = fmod(position.x(), GRID_SIZE);
	const float eastWallDistance  = GRID_SIZE - westWallDistance;
	const float southWallDistance = GRID_SIZE - northWallDistance;
	
	/*const float northWestDistance = sqrt(northWallDistance*northWallDistance + westWallDistance*westWallDistance);
	const float northEastDistance = sqrt(northWallDistance*northWallDistance + eastWallDistance*eastWallDistance);
	const float southEastDistance = sqrt(southWallDistance*southWallDistance + eastWallDistance*eastWallDistance);
	const float southWestDistance = sqrt(southWallDistance*southWallDistance + westWallDistance*westWallDistance);*/
	
	if (northWall && displacement.y() < 0)
	{
		if (-displacement.y() + PLAYER_RADIUS >= northWallDistance)
			displacement.setY(-(northWallDistance - PLAYER_RADIUS));
	}
	else if (southWall && displacement.y() > 0)
	{
		if (displacement.y() + PLAYER_RADIUS >= southWallDistance)
			displacement.setY(southWallDistance - PLAYER_RADIUS);
	}
	if (westWall && displacement.x() < 0)
	{
		if (-displacement.x() + PLAYER_RADIUS >= westWallDistance)
			displacement.setX(-(westWallDistance - PLAYER_RADIUS));
	}
	else if (eastWall && displacement.x() > 0)
	{
		if (displacement.x() + PLAYER_RADIUS >= eastWallDistance)
			displacement.setX(eastWallDistance - PLAYER_RADIUS);
	}
	
	const QPointF hitboxCenter = QPointF(westWallDistance, northWallDistance);
	const QPointF cornerPoints[] =
	{
		QPointF(0.0, 0.0),
		QPointF(GRID_SIZE, 0.0),
		QPointF(GRID_SIZE, GRID_SIZE),
		QPointF(0.0, GRID_SIZE)
	};
	const bool cornerExists[] =
	{
		northWestCorner,
		northEastCorner,
		southEastCorner,
		southWestCorner
	};
	const int displacementLength = QLineF(position, position + displacement).length();
	for (unsigned int i = 0; i < ARRAYSIZE(cornerPoints); i++)
	{
		if (cornerExists[i])
		{
			QLineF line(cornerPoints[i], hitboxCenter + displacement);
			const float lineLength = line.length();
			if (lineLength <= PLAYER_RADIUS)
			{
				line.setLength(PLAYER_RADIUS);
				float swingAround = (PLAYER_RADIUS - lineLength)/(3.14592*PLAYER_RADIUS)*180.0/4.0;
				QLineF line2 = line;
				line2.setAngle(line.angle() - swingAround);
				line.setAngle(line.angle() + swingAround);
				line.setP1(hitboxCenter);
				line2.setP1(hitboxCenter);
				if (line.length() > line2.length())
					return position + line.p2() - hitboxCenter;
				else
					return position + line2.p2() - hitboxCenter;
			}
		}
	}
	
	const QPointF result = position + displacement;
	return result;
}

void Maze::drawGrid()
{
	// glColor3f(0.85, 0.85, 0.85);
	// glColor3f(0.0, 0.0, 0.0);
	glColor3f(0.5, 0.5, 0.5);
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

void Maze::draw()
{
	_DrawFloor();
	_DrawCeiling();
	_DrawWalls();
	_DrawEditorWallTops();

	// draw the tops of the pictures for the editor's benefit
	if (dropping.dragging == Dropping::DRAGGING_IMAGE && containsTile(dropping.orientation.tile))
		dropping.orientation.draw();
	else if (dropping.dragging == Dropping::DRAGGING_GOAL && containsTile(dropping.orientation.tile))
		_DrawGoal(dropping.orientation.tile);
	
	// draw pictures
	pictures.draw();
	
	// draw the starting location
	glColor3f(0.0, 1.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(startingOrientation.tile.x()*GRID_SIZE + GRID_SIZE/2.0, startingOrientation.tile.y()*GRID_SIZE + GRID_SIZE/2.0, -0.3);
	switch (startingOrientation.direction)
	{
		case Orientation::North:
		break;
		
		case Orientation::South:
		glRotatef(180.0, 0.0, 0.0, 1.0);
		break;
		
		case Orientation::East:
		glRotatef(90.0, 0.0, 0.0, 1.0);
		break;
		
		case Orientation::West:
		glRotatef(270.0, 0.0, 0.0, 1.0);
		break;
	}
	glBegin(GL_TRIANGLES);
	{
		static const int SPACING = 20;
		glVertex3i(-GRID_SIZE/2.0 + SPACING, -GRID_SIZE/2.0 + SPACING, 0);
		glVertex3i(0, 0, 0);
		glVertex3i(GRID_SIZE/2.0 - SPACING, -GRID_SIZE/2.0 + SPACING, 0);
	}
	glEnd();
	glPopMatrix();
	
	// draw the goals
	for (QSet<QPoint>::const_iterator it = goalTiles.begin(); it != goalTiles.end(); ++it)
		_DrawGoal(*it);
	
	// glBindTexture(GL_TEXTURE_2D, 0);
}

void Maze::_DrawGoal(const QPoint &goal)
{
	glColor3f(1.0, 0.0, 1.0);
	{
		GLUquadric * const quadric = gluNewQuadric();
		{
			glPushMatrix();
			glTranslatef(goal.x()*GRID_SIZE + GRID_SIZE/2.0, goal.y()*GRID_SIZE + GRID_SIZE/2.0, -0.4);
			glRotatef(180.0, 1.0, 0.0, 0.0);
			// gluDisk(quadric, 30.0, 15.0, 32, 2);
			gluCylinder(quadric, 30.0, 0.0, GRID_SIZE, 32, 1);
			glPopMatrix();
		}
		gluDeleteQuadric(quadric);
	}
	glColor3f(1.0, 1.0, 1.0);
}

#include <cstdio>

#include <QStack>
void Maze::_RefreshTiles()
{
	tiles.fill(0);
	
	QStack<QPoint> toFill;
	toFill.push(QPoint(startingOrientation.tile.x(), startingOrientation.tile.y()));
	while (!toFill.empty())
	{
		const QPoint p = toFill.pop();
		if (!containsTile(p))
			continue;
		if (tileAt(p))
			continue;
		
		tileAt(p) = true;
		
		if (!(vertexAt(p) & EastWall))
			toFill.push(p + QPoint(0, -1));
		if (!(vertexAt(p + QPoint(0, 1)) & EastWall))
			toFill.push(p + QPoint(0, 1));
		if (!(vertexAt(p) & SouthWall))
			toFill.push(p + QPoint(-1, 0));
		if (!(vertexAt(p + QPoint(1, 0)) & SouthWall))
			toFill.push(p + QPoint(1, 0));
	}
}

void Maze::_DrawWalls()
{
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glBegin(GL_QUADS);
	{
		int i = 0;
		for (int row = 0; row <= height; row++)
		{
			for (int col = 0; col <= width; col++, i++)
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
					if (walls[i+(width+1)] == NorthEastCorner)
						bottomOffsetLeft = -HALF_WALL_WIDTH;
					
					float topOffsetRight = (walls[i] & EastWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i] == SouthWestCorner)
						topOffsetRight = -HALF_WALL_WIDTH;
					float bottomOffsetRight = (walls[i+(width+1)] & EastWall) ? HALF_WALL_WIDTH : 0.0;
					if (walls[i+(width+1)] == NorthWestCorner)
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
}

void Maze::_DrawCeiling()
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
}

void Maze::_DrawFloor()
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
}

void Maze::_DrawEditorWallTops()
{
	glColor3f(0.0, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_QUADS);
	{
		int i = 0;
		for (int row = 0; row <= height; row++)
		{
			for (int col = 0; col <= width; col++, i++)
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
