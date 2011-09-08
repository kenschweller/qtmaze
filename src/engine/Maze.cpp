#include "Maze.h"
#include "defines.h"

#include <QGLWidget>
#include <QFile>
#include <QTextStream>
#include <QLineF>

#include <cstdlib>
#include <cmath>

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

uint qHash(const QPoint &p)
{
	return (p.y() << 16) + p.x();
}

Maze::Maze() : context(NULL)
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
	tiles.resize(width, height);
	walls.resize(width, height);
	tiles.clear();
	walls.clear();
	startingOrientation = Orientation();
	goalTiles.clear();
	goalTiles.insert(QPoint(0, 0));
	pictures.clear();
	_RefreshTiles();
}
// #include <QDebug>
bool Maze::load(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	int newWidth = 0;
	int newHeight = 0;
	int newSize = 0;
	Walls newWalls;
	GoalSet newGoalTiles;
	Pictures newPictures;
	newWalls.setContext(context);
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
		else if (sectionName == "walls")
		{
			if (!newWalls.read(file))
				return false;
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
		else if (sectionName == "walltextures")
		{
			if (!newWalls.readTextures(file))
				return false;
		}
		else if (sectionName == "walltexture")
		{
			if (words.size() < 3)
				return false;
			bool ok = false;
			const quint8 id = words[1].toUInt(&ok, 16);
			if (!ok)
				return false;
			const int filenameStart = words[2].indexOf('"') + 1;
			const int filenameEnd = words[2].lastIndexOf('"');
			const QString filename = words[2].mid(filenameStart, filenameEnd - filenameStart);
			newWalls.textures.mapTexture(id, filename);
			// qDebug() << "Mapping" << id << " --" << filename;
		}

		// use the new dimensions
		if (newWidth > 0 && newHeight > 0 && newSize != newWidth*newHeight)
		{
			newSize = newWidth*newHeight;
			newWalls.clear();
			newWalls.resize(newWidth, newHeight);
		}
	}

	if (newWidth < 1 || newHeight < 1)
		return false;

	width = newWidth;
	height = newHeight;
	tiles.resize(width, height);
	tiles.clear();
	walls = newWalls;
	goalTiles = newGoalTiles;
	if (goalTiles.empty())
		goalTiles.insert(QPoint(0, 0));
	startingOrientation = newStartingOrientation;
	pictures = newPictures;

	_RefreshTiles();

	return true;
}

// static char EncodeNumberToChar(const quint8 id)
// {
	// if (id < 10)
		// return ('0' + id);
	// else if (id < 36)
		// return ('A' + id);
	// else
		// return '0';
// }

static QString EncodeNumberToChar(const quint8 id)
{
	return QString::number(id, 16).rightJustified(2, '0');
}

bool Maze::save(const QString &filename) const
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QFile::Truncate))
		return false;

	QTextStream out(&file);
	out << "width " << width << "\n";
	out << "height " << height << "\n";

	out << "walls\n";
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if ((walls.at(row, col) & Walls::SouthEastCorner) == Walls::SouthEastCorner)
				out << "+";
			else if (walls.at(row, col) & Walls::EastWall)
				out << "-";
			else if (walls.at(row, col) & Walls::SouthWall)
				out << "|";
			else
				out << ".";
		}
		out << "\n";
	}
	out << "walltextures\n";
	for (int row = 0; row < walls.height; row++)
	{
		for (int col = 0; col < walls.width; col++)
		{
			const Walls::TextureIDs & corner = walls.internalTextureIdAt(row, col);
			out << EncodeNumberToChar(corner.vertex);
			out << EncodeNumberToChar(corner.east_northeast);
			out << EncodeNumberToChar(corner.east_southeast);
			out << EncodeNumberToChar(corner.south_southwest);
			out << EncodeNumberToChar(corner.south_southeast);
		}
		out << "\n";
	}

	out << "start row " << startingOrientation.tile.y() << " col " << startingOrientation.tile.x() << " side " << startingOrientation.directionToString() << "\n";

	for (GoalSet::const_iterator it = goalTiles.begin(); it != goalTiles.end(); ++it)
	{
		out << "goal row " << it->y() << " col " << it->x() << "\n";
	}

	out << pictures;

	for (WallTextures::IdToNameMap::const_iterator it = walls.wallTextures().textureNames().begin(); it != walls.wallTextures().textureNames().end(); ++it)
	{
		out << "walltexture " << it.key() << " \"" << it.value() << "\"\n";
	}

	return true;
}

void Maze::setContext(QGLWidget *c)
{
	context = c;

	tiles.setContext(c);
	walls.setContext(c);
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

bool Maze::mapPointInGoalRadius(const QPoint &point) const
{
	const QPoint tile = getNearestTile(point);
	if (!containsTile(tile))
		return false;
	if (!goalTiles.contains(tile))
		return false;

	const QLineF line(tile.x()*GRID_SIZE+GRID_SIZE/2.0, tile.y()*GRID_SIZE+GRID_SIZE/2.0, point.x(), point.y());

	return line.length() <= GOAL_RADIUS;
}

QPoint Maze::getNearestVertex(const QPoint &point) const
{
	return point/GRID_SIZE;
}

QPoint Maze::getNearestTile(const QPoint &point) const
{
	return (point - QPoint(GRID_SIZE/2, GRID_SIZE/2))/GRID_SIZE;
}

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

void Maze::addPicture(const Orientation &orientation, const QString &filename)
{
	if (!containsTile(orientation.tile))
		return;
	removePicture(orientation);
	pictures.add(orientation, filename);
}

void Maze::addGoal(const QPoint &p)
{
	if (containsTile(p))
		goalTiles.insert(p);
}

void Maze::setStartingOrientation(const Orientation &orientation)
{
	if (containsTile(orientation.tile))
	{
		startingOrientation = orientation;
		_RefreshTiles();
	}
}

bool Maze::connectVertices(const QPoint &a, const QPoint &b)
{
	if (!walls.addWallBetweenVertices(a, b))
		return false;
	_RefreshTiles();
	return true;
}

void Maze::disconnectVertices(const QPoint &a, const QPoint &b)
{
	walls.removeWallBetweenVertices(a, b);
	_RefreshTiles();
}

void Maze::removeWall(const QPoint &a, const QPoint &b)
{
	walls.removeWallBetweenTiles(a, b);
	_RefreshTiles();
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

void Maze::startPaintingWallTexture(const QString &filename)
{
	dropping.dragging = Dropping::DRAGGING_WALLTEXTURE;
	dropping.orientation.tile = QPoint(-1, -1);
	dropping.filename = filename;
}

void Maze::continuePaintingWallTexture(const QPoint &p)
{
	dropping.orientation = getNearestOrientation(p);
	if (containsTile(dropping.orientation.tile))
		walls.paintWall(dropping.orientation, dropping.filename);
}

void Maze::endPaintingWallTexture(bool dropped)
{
	dropping.dragging = Dropping::DRAGGING_NONE;
}

QPointF Maze::addDisplacement(const QPointF &position, QPointF displacement) const
{
	const QPoint tile = getNearestTile(position.toPoint());
	bool northWall = false, eastWall = false, southWall = false, westWall = false;
	bool northWestCorner = false, northEastCorner = false, southEastCorner = false, southWestCorner = false;
	if (containsTile(tile))
	{
		northWall = walls.at(tile)                & Walls::EastWall;
		eastWall  = walls.at(tile + QPoint(1, 0)) & Walls::SouthWall;
		southWall = walls.at(tile + QPoint(0, 1)) & Walls::EastWall;
		westWall  = walls.at(tile)                & Walls::SouthWall;

		northWestCorner = walls.at(tile)                & Walls::NorthWestCorner;
		northEastCorner = walls.at(tile + QPoint(1, 0)) & Walls::NorthEastCorner;
		southEastCorner = walls.at(tile + QPoint(1, 1)) & Walls::SouthEastCorner;
		southWestCorner = walls.at(tile + QPoint(0, 1)) & Walls::SouthWestCorner;
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
	// const int displacementLength = QLineF(position, position + displacement).length();
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

void Maze::drawGrid() const
{
	tiles.drawGrid();
}

void Maze::draw() const
{
	tiles.drawFloor();
	tiles.drawCeiling();
	walls.draw();
	walls.drawTops();
	pictures.draw();

	/*glEnable(GL_LIGHTING);
	static GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	static GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat LightPosition[] = { 0.0f, 0.0f, -100.0f, 1.0f };
	// static GLfloat LightPosition[] = {1.0, 0.5, 1.0, 0.0};
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT1);*/

	// draw 'temporary' elements that we are currently dragging around
	if (dropping.dragging == Dropping::DRAGGING_IMAGE && containsTile(dropping.orientation.tile))
		dropping.orientation.draw();
	if (dropping.dragging == Dropping::DRAGGING_GOAL && containsTile(dropping.orientation.tile))
		_DrawGoal(dropping.orientation.tile);

	_DrawStartingOrientation();
	_DrawGoals();

	/*glDisable(GL_LIGHTING);*/
}

void Maze::_DrawStartingOrientation() const
{
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
	glColor3f(1.0, 1.0, 1.0);
}

void Maze::_DrawGoals() const
{
	for (GoalSet::const_iterator it = goalTiles.begin(); it != goalTiles.end(); ++it)
		_DrawGoal(*it);
}

void Maze::_DrawGoal(const QPoint &goal) const
{
	glColor3f(1.0, 0.0, 1.0);
	{
		GLUquadric * const quadric = gluNewQuadric();
		{
			glPushMatrix();
			glTranslatef(goal.x()*GRID_SIZE + GRID_SIZE/2.0, goal.y()*GRID_SIZE + GRID_SIZE/2.0, -0.4);
			glRotatef(180.0, 1.0, 0.0, 0.0);
			// gluDisk(quadric, 30.0, 15.0, 32, 2);
			gluCylinder(quadric, GOAL_RADIUS, 0.0, GRID_SIZE, 32, 1);
			glPopMatrix();
		}
		gluDeleteQuadric(quadric);
	}
	glColor3f(1.0, 1.0, 1.0);
}

void Maze::_RefreshTiles()
{
	tiles.clear();
	tiles.floodFill(startingOrientation.tile, walls);
}
