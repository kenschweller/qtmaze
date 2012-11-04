#include "MazeWidget.h"
#include "engine/Path.h"
#include "engine/defines.h"

#include <QApplication>
#include <QFileInfo>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVector3D>
#include <QThread>
#include <QSound>
#include <QDir>
#include <QLineF>
#include <QDebug>

#include <cmath>

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) sizeof(x)/sizeof(x[0])
#endif

static const int RESIZE_THRESHOLD = 5;

enum Holding
{
	HOLDING_FORWARD,
	HOLDING_BACKWARD,
	HOLDING_STRAFELEFT,
	HOLDING_STRAFERIGHT,
	HOLDING_TURNLEFT,
	HOLDING_TURNRIGHT
};

QPoint MazeWidget3D::_Unproject(const QPoint &point, bool wallsToo)
{
	Q_UNUSED(wallsToo);
	GLdouble modelviewMatrix[16];
	GLdouble projectionMatrix[16];
	GLint viewport[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble x, y, z;
	GLdouble x2, y2, z2;
	gluUnProject(point.x(), height() - point.y(), 1.0, modelviewMatrix, projectionMatrix, viewport, &x, &y, &z);
	gluUnProject(point.x(), height() - point.y(), 0.0, modelviewMatrix, projectionMatrix, viewport, &x2, &y2, &z2);

	const QVector3D rayEnd(x, y, z);
	const QVector3D rayStart(x2, y2, z2);
	const QVector3D rayDirection = (rayEnd - rayStart).normalized();

	// const QVector3D mapCenter = QVector3D(maze.getWidth()/2, maze.getHeight()/2, -GRID_SIZE/2);// + tumble.position;
	//QVector3D viewFrom = mapCenter - tumble.view*(maze.getWidth() > maze.getHeight() ? maze.getWidth() : maze.getHeight())*1.3*scaleFactor;

	if (mode == MODE_OVERVIEW || mode == MODE_MOUSELOOK)
	{
		// TODO make this not always interesect with the floor, but
		// rather test the walls and such that the ray passes through
		const QVector3D intersection = rayStart + rayDirection*abs(rayStart.z()/rayDirection.z());
		return intersection.toPoint();
	}
	else
		return rayEnd.toPoint();
}

MazeWidget3D::MazeWidget3D(QWidget *parent)
	: QGLWidget(parent),
	mode(MODE_EDITING), occupation(OCCUPATION_NONE),
	scaleFactor(1.0), editingScaleFactor(3.0),
	joystick(NULL),
	verticalOffset(0), horizontalOffset(0),
	displayListMaze(0),
	goalSound("data/sounds/goal.wav"),
	playing(false), won(false),
	joystickDeadZone(0.05), joystickWalkingSpeed(1.0), joystickTurningSpeed(1.0), currentWallHeight(10), paintingWallsMode(BULLDOZING_PLACING_WALLS)
{
	_participating = false;
	camera = maze.getStartingCamera();
	tumble.view = QVector3D(0.0, 0.0, 1.0);
	tumble.up = QVector3D(0.0, -1.0, 0.0);

	setMouseTracking(mode == MODE_EDITING);
	for (unsigned int i = 0; i < ARRAYSIZE(holdingKeys); i++)
		holdingKeys[i] = 0;

	connect(&timer, SIGNAL(timeout()), this, SLOT(slot_Advance()));
	timer.setInterval(10);
	timer.start();

	setAcceptDrops(true);

	// grabKeyboard();
	setFocusPolicy(Qt::StrongFocus);

	/*{
		// Print the names of all attached joysticks
		const int num_joy = SDL_NumJoysticks();
		printf("%d joysticks found\n", num_joy);
		for(int i=0;i<num_joy;i++)
			printf("%d: %s\n", i, SDL_JoystickName(i));
	}*/

	quadric = gluNewQuadric();

	if (SDL_NumJoysticks() > 0)
		joystick = SDL_JoystickOpen(0);
	// printf("joystick = %x\n", (int)joystick);

	oldMaze.reset(0, 0);
	reset(10, 10);
}

MazeWidget3D::~MazeWidget3D()
{
	logger.end();
	gluDeleteQuadric(quadric);
	// crashes for whatever reason
	// if (joystick && SDL_JoystickOpened(0))
		// SDL_JoystickClose(joystick);
}

bool MazeWidget3D::isParticipating() const
{
	return _participating && _participantName.size() > 0 && _filename.size() > 0;
}

void MazeWidget3D::setParticipantName(const QString &name)
{
	_participantName = name;
}

void MazeWidget3D::setParticipating(bool enabled)
{
	_participating = enabled;
}

void MazeWidget3D::reset(int width, int height/*, const QString description*/)
{
	_filename = "";
	maze.reset(width, height);
	// maze.setDescription(description); // TODO
	restart(false);
	paths.clear();
}

void MazeWidget3D::_RestartLogging()
{
	logger.end();
	if (isParticipating())
	{
		const QFileInfo mazeFileInfo(_filename);
		const QString basePath("logs/" + _participantName + "/" + mazeFileInfo.completeBaseName());
		QDir::current().mkpath(basePath);
		QString filename;
		for (int i = 0; i < 9999; i++)
		{
			filename = basePath + "/" + QString::number(i).rightJustified(4, '0') + ".txt";
			if (!QFile::exists(filename))
				break;
		}
		logger.start(filename, _participantName, QDir::current().relativeFilePath(mazeFileInfo.filePath()));
	}
}

bool MazeWidget3D::open(const QString &filename, bool pplaying)
{
	const bool result = maze.load(filename);
	if (result)
	{
		_filename = filename;
		restart(pplaying);
		paths.clear();
	}
	return result;
}

bool MazeWidget3D::save(const QString &filename)
{
	const bool result = maze.save(filename);
	if (result)
		_filename = filename;
	return result;
}

QString MazeWidget3D::getFilename() const
{
	return _filename;
}

void MazeWidget3D::slot_SwitchToEditingMode()
{
	if (mode != MODE_EDITING)
	{
		mode = MODE_EDITING;
		_SetupProjectionMatrix();
		_SetupModelMatrix();
		update();
		setMouseTracking(true);
	}
}

void MazeWidget3D::slot_SwitchToOverviewMode()
{
	if (mode != MODE_OVERVIEW)
	{
		mode = MODE_OVERVIEW;
		_SetupProjectionMatrix();
		_SetupModelMatrix();
		update();
		setMouseTracking(false);
	}
}

void MazeWidget3D::slot_SwitchToMouselookMode()
{
	if (mode != MODE_MOUSELOOK)
	{
		mode = MODE_MOUSELOOK;
		_SetupProjectionMatrix();
		_SetupModelMatrix();
		update();
		setMouseTracking(false);
	}
}

void MazeWidget3D::slot_SwitchToWallPlacingMode()
{
	paintingWallsMode = BULLDOZING_PLACING_WALLS;
}

void MazeWidget3D::slot_SwitchToWallPaintingMode()
{
	paintingWallsMode = BULLDOZING_PAINTING_WALLS;
}

void MazeWidget3D::slot_SwitchToWallHeightSettingMode()
{
	qDebug() << "Switched to height setting mode";
	paintingWallsMode = BULLDOZING_SETTING_WALL_HEIGHT;
}

void MazeWidget3D::restart(bool pplaying)
{
	playing = pplaying;
	won = false;
	_RestartLogging();
	horizontalOffset = maze.getWidth()/2.0;
	verticalOffset = maze.getHeight()/2.0;
	overviewHorizontalOffset = maze.getWidth()/2.0;
	overviewVerticalOffset = maze.getHeight()/2.0;
	camera = maze.getStartingCamera();
	_SetupProjectionMatrix();
	_SetupModelMatrix();
	update();
	emit(zoomChanged());
	if (pplaying)
		paths.clear();
}

void MazeWidget3D::addPath(const Path &path)
{
	paths.push_back(path);
	update();
}

void MazeWidget3D::clearPaths()
{
	paths.clear();
	update();
}

QVector3D MazeWidget3D::_GetKeyboardDirection()
{
	float forward_dir = 0.0;
	float   right_dir = 0.0;
	if (holdingKeys[HOLDING_FORWARD]) forward_dir += 1.0;
	if (holdingKeys[HOLDING_BACKWARD]) forward_dir -= 1.0;
	if (holdingKeys[HOLDING_STRAFELEFT]) right_dir -= 1.0;
	if (holdingKeys[HOLDING_STRAFERIGHT]) right_dir += 1.0;

	return ((camera.getForward() * forward_dir) + (camera.getRight() * right_dir)).normalized();
}

static const float JOYSTICK_AXIS_DIVISOR = 65536.0/2.0;

QVector3D MazeWidget3D::_GetJoystickDirection()
{
	float y_axis = -static_cast<float>(SDL_JoystickGetAxis(joystick, 1));
	if (fabs(y_axis) <= joystickDeadZone)
		y_axis = 0.0;
	return camera.getForward() * y_axis/JOYSTICK_AXIS_DIVISOR;
}

float MazeWidget3D::_GetKeyboardTurning()
{
	float dx = 0.0;
	if (holdingKeys[HOLDING_TURNLEFT]) dx -= 1.0;
	if (holdingKeys[HOLDING_TURNRIGHT]) dx += 1.0;
	return dx;
}

float MazeWidget3D::_GetJoystickTurning()
{
	float x_axis = static_cast<float>(SDL_JoystickGetAxis(joystick, 0))/JOYSTICK_AXIS_DIVISOR;
	if (fabs(x_axis) <= joystickDeadZone)
		x_axis = 0.0;
	return x_axis;
}

void MazeWidget3D::slot_Advance()
{
	if (won || (mode != MODE_MOUSELOOK && mode != MODE_OVERVIEW))
		return;

	if (joystick)
		SDL_JoystickUpdate();

	static const float KEYBOARD_WALKING_SPEED = 10.0;
	static const float KEYBOARD_TURNING_SPEED = 1.0;

	const float turning          = _GetKeyboardTurning()  *KEYBOARD_TURNING_SPEED + _GetJoystickTurning()  *joystickTurningSpeed;
	const QVector3D displacement = _GetKeyboardDirection()*KEYBOARD_WALKING_SPEED + _GetJoystickDirection()*joystickWalkingSpeed;

	if (displacement.isNull() && turning == 0.0)
		return;

	const QPointF movedPoint = maze.addDisplacement(camera.position.toPointF(), displacement.toPointF());
	camera.position = QVector3D(movedPoint.x(), movedPoint.y(), camera.position.z());
	const QLineF viewLine(0.0, 0.0, camera.view.x(), camera.view.y());
	logger.log(camera.position.x(), camera.position.y(), viewLine.angle());

	if (mode == MODE_MOUSELOOK)
		camera.moveView(turning, 0.0);

	_SetupProjectionMatrix();
	_SetupModelMatrix();
	update();

	if (playing && maze.mapPointInGoalRadius(camera.position.toPoint()))
	{
		goalSound.play();
		playing = false;
		won = true;
		logger.end();
		update();
		QTimer::singleShot(1000, this, SLOT(slot_SoundFinished()));
	}
}

void MazeWidget3D::slot_SoundFinished()
{
	emit(completedMaze(_filename));
}

void MazeWidget3D::dragEnterEvent(QDragEnterEvent *event)
{
	const QPoint mapPoint = _Unproject(event->pos());

	if (event->mimeData()->hasFormat("application/x-qtmaze-startingorientation"))
	{
		event->acceptProposedAction();

		const Orientation orientation = maze.getNearestOrientation(mapPoint);
		maze.setStartingOrientation(orientation);
		update();
		return;
	}
	else if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
		QDataStream stream(&encoded, QIODevice::ReadOnly);
		if (!stream.atEnd())
		{
			int row, col;
			QMap<int, QVariant> roleDataMap;
			stream >> row >> col >> roleDataMap;
			const QString filename = roleDataMap[Qt::DisplayRole].toString();
			// printf("\tDragging '%s' to (%i, %i)\n", roleDataMap[Qt::DisplayRole].toString().toAscii().data(), event->pos().x(), event->pos().y());
			event->acceptProposedAction();
			maze.startDroppingImage(filename);
			maze.moveDroppingImage(mapPoint);
			update();
		}
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-newgoal"))
	{
		event->acceptProposedAction();
		maze.startDroppingGoal();
		maze.moveDroppingGoal(mapPoint);
		update();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-cameraposition"))
	{
		event->acceptProposedAction();
	}
}

void MazeWidget3D::dragLeaveEvent(QDragLeaveEvent *event)
{
	Q_UNUSED(event);
	maze.endDroppingImage(false);
	maze.endDroppingGoal(false);
	update();
}

void MazeWidget3D::dragMoveEvent(QDragMoveEvent *event)
{
	const QPoint mapPoint = _Unproject(event->pos());
	if (event->mimeData()->hasFormat("application/x-qtmaze-startingorientation"))
	{
		const Orientation orientation = maze.getNearestOrientation(mapPoint);
		maze.setStartingOrientation(orientation);
		update();
		return;
	}
	else if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		// printf("Drag moving to (%i, %i)\n", event->pos().x(), event->pos().y());
		maze.moveDroppingImage(mapPoint);
		update();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-cameraposition"))
	{
		camera.position.setX(mapPoint.x());
		camera.position.setY(mapPoint.y());
		update();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-newgoal"))
	{
		maze.moveDroppingGoal(mapPoint);
		update();
	}
}

void MazeWidget3D::dropEvent(QDropEvent *event)
{
	const QPoint mapPoint = _Unproject(event->pos());
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		maze.endDroppingImage(true);
		update();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-startingorientation"))
	{
		const Orientation orientation = maze.getNearestOrientation(mapPoint);
		maze.setStartingOrientation(orientation);
		update();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-newgoal"))
	{
		maze.moveDroppingGoal(mapPoint);
		maze.endDroppingGoal(true);
		update();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-cameraposition"))
	{
		camera.position.setX(mapPoint.x());
		camera.position.setY(mapPoint.y());
		update();
	}
}

void MazeWidget3D::focusOutEvent(QFocusEvent *event)
{
	for (unsigned int i = 0; i < ARRAYSIZE(holdingKeys); i++)
		holdingKeys[i] = false;
	QGLWidget::focusOutEvent(event);
}

#include <QMenu>
void MazeWidget3D::contextMenuEvent(QContextMenuEvent *event)
{
	const QPoint mapPoint = _Unproject(event->pos());
	const Orientation orientation = maze.getNearestOrientation(mapPoint);
	const QPoint tile = orientation.tile;

	event->accept();
	QMenu menu;
	QAction * const goalAction = menu.addAction("Add Goal");
	QAction * const startAction = menu.addAction("Set Start");
	QAction * const placeAction = menu.addAction("Place Camera");
	QAction * const selected = menu.exec(event->globalPos());
	if (selected == goalAction)
	{
		maze.addGoal(tile);
		update();
	}
	else if (selected == startAction)
	{
		maze.setStartingOrientation(orientation);
		update();
	}
	else if (selected == placeAction)
	{
		camera.position.setX(mapPoint.x());
		camera.position.setY(mapPoint.y());
		_SetupModelMatrix();
		_SetupProjectionMatrix();
		update();
	}
}

void MazeWidget3D::mousePressEvent(QMouseEvent *event)
{
	if (occupation != OCCUPATION_NONE)
		return;
	lastPos = event->pos();
	if (event->button() != Qt::LeftButton)
		return;

	if (mode == MODE_OVERVIEW && (event->modifiers() & Qt::AltModifier))
		return;
	const QPoint mapPoint = _Unproject(event->pos());
	if (mode == MODE_EDITING)
	{
		const Orientation orientation = maze.getNearestOrientation(mapPoint);
		const QString filename = maze.getPicture(orientation).filename;
		if (QLineF(camera.position.toPoint(), mapPoint).length() < CAMERA_RADIUS)
		{
			QDrag drag(this);
			{
				QMimeData * const mimeData = new QMimeData;
				mimeData->setData("application/x-qtmaze-cameraposition", QByteArray());
				drag.setMimeData(mimeData);
			}
			drag.start(Qt::CopyAction | Qt::MoveAction);
		}
		else if (filename.size() > 0)
		{
			maze.removePicture(orientation);
			QDrag drag(this);

			{
				QByteArray encoded;
				QDataStream stream(&encoded, QIODevice::WriteOnly);
				int row = 0, col = 0;
				QMap<int, QVariant> roleDataMap;
				roleDataMap[Qt::DisplayRole] = filename;
				roleDataMap[Qt::DecorationRole] = 0;
				stream << row << col << roleDataMap;
				{
					QMimeData * const mimeData = new QMimeData;
					mimeData->setData("application/x-qabstractitemmodeldatalist", encoded);
					drag.setMimeData(mimeData);
				}
			}

			drag.start(Qt::CopyAction | Qt::MoveAction);
		}
		else if (maze.mapPointInGoalRadius(mapPoint))
		{
			maze.removeGoal(orientation.tile);
			QDrag drag(this);
			{
				QMimeData * const mimeData = new QMimeData;
				mimeData->setData("application/x-qtmaze-newgoal", QByteArray());
				drag.setMimeData(mimeData);
			}
			drag.start(Qt::CopyAction | Qt::MoveAction);
		}
		else if (orientation == maze.getStartingOrientation())
		{
			QDrag drag(this);
			{
				QMimeData * const mimeData = new QMimeData;
				mimeData->setData("application/x-qtmaze-startingorientation", QByteArray());
				drag.setMimeData(mimeData);
			}
			drag.start(Qt::CopyAction | Qt::MoveAction);
		}
		else
		{
			goto normal_click;
		}
		return;
	}

	normal_click:
	if (paintingWallsMode == BULLDOZING_PAINTING_WALLS)
	{
		occupation = OCCUPATION_PAINTING;
		if (currentWallTextureFilename.size() == 0) // TODO necessary?
			currentWallTextureFilename = "wall.jpg";
		maze.startPaintingWallTexture(currentWallTextureFilename);
		maze.continuePaintingWallTexture(mapPoint);
		update();
	}
	else if (paintingWallsMode == BULLDOZING_PLACING_WALLS)
	{
		occupation = OCCUPATION_DRAWING;
		drawPoints.clear();
		drawPoints.append(mapPoint);
		if (event->modifiers() & Qt::ShiftModifier)
			lastTilePos = maze.getNearestTile(mapPoint);
		else
			lastTilePos = maze.getNearestVertex(mapPoint);
	}
	else if (paintingWallsMode == BULLDOZING_SETTING_WALL_HEIGHT)
	{
		occupation = OCCUPATION_SETTINGWALLHEIGHT;
		maze.startSettingWallHeight(currentWallHeight);
		maze.continueSettingWallHeight(mapPoint);
		update();
	}
}

void MazeWidget3D::mouseReleaseEvent(QMouseEvent *event)
{
	lastPos = event->pos();
	if (event->button() != Qt::LeftButton)
		return;
	if (occupation == OCCUPATION_DRAWING)
	{
		drawPoints.clear();
		update();
		occupation = OCCUPATION_NONE;
	}
	else if (occupation == OCCUPATION_PAINTING)
	{
		maze.endPaintingWallTexture(true);
		occupation = OCCUPATION_NONE;
	}
	else if (occupation == OCCUPATION_SETTINGWALLHEIGHT)
	{
		maze.endSettingWallHeight(true);
		occupation = OCCUPATION_NONE;
	}
}
// #include <cstdio>
void MazeWidget3D::mouseMoveEvent(QMouseEvent *event)
{
	static const float DIVISOR = 20;

	const QPoint newPos = event->pos();
	const float dx = static_cast<float>(newPos.x() - lastPos.x())/DIVISOR;
	const float dy = static_cast<float>(newPos.y() - lastPos.y())/DIVISOR;
	lastPos = event->pos();

	if ((mode == MODE_MOUSELOOK || mode == MODE_OVERVIEW) && occupation == OCCUPATION_NONE)
	{
		if (mode == MODE_MOUSELOOK)
			camera.moveView(-dx, -dy, false);
		else
			tumble.moveView(dx*2, dy*2, true);
		_SetupModelMatrix();
		update();
	}
	else if (occupation == OCCUPATION_DRAWING)
	{
		const QPoint mapPoint = _Unproject(event->pos());
		if (event->modifiers() & Qt::ShiftModifier)
		{
			const QPoint curTilePos = maze.getNearestTile(mapPoint);
			maze.removeWall(lastTilePos, curTilePos);
			lastTilePos = curTilePos;
		}
		else
		{
			const QPoint curTilePos = maze.getNearestVertex(mapPoint);
			maze.connectVertices(lastTilePos, curTilePos);
			lastTilePos = curTilePos;
		}
		drawPoints.append(mapPoint);
		update();
	}
	else if (occupation == OCCUPATION_PAINTING)
	{
		const QPoint mapPoint = _Unproject(event->pos());
		maze.continuePaintingWallTexture(mapPoint);
		update();
	}
}

void MazeWidget3D::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	if (mode != MODE_MOUSELOOK)
		return;
	levelView();
}

void MazeWidget3D::levelView()
{
	if (camera.view.z() == 0.0)
		return;
	camera.view.setZ(0);
	camera.view.normalize();

	_SetupProjectionMatrix();
	_SetupModelMatrix();
	update();
}

void MazeWidget3D::wheelEvent(QWheelEvent *event)
{
	// int numDegrees = event->delta() / 8;
	// int numSteps = numDegrees / 15;
	if (mode == MODE_OVERVIEW)
	{
		if (event->orientation() == Qt::Vertical)
		{
			if (event->delta() < 0.0)
				scaleFactor *= 1.1;
			else
				scaleFactor /= 1.1;
			_SetupModelMatrix();
			update();
			emit(zoomChanged());
		}

		event->accept();
	}
	else if (mode == MODE_EDITING)
	{
		if (event->orientation() == Qt::Vertical)
		{
			if (event->delta() < 0.0)
				editingScaleFactor *= 1.1;
			else if (editingScaleFactor > 1.0)
				editingScaleFactor /= 1.1;
			_SetupProjectionMatrix();
			_SetupModelMatrix();
			update();
			emit(zoomChanged());
		}

		event->accept();
	}
}

void MazeWidget3D::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Up:
		case Qt::Key_W: holdingKeys[HOLDING_FORWARD]++; break;
		case Qt::Key_Down:
		case Qt::Key_S: holdingKeys[HOLDING_BACKWARD]++; break;
		case Qt::Key_A: holdingKeys[HOLDING_STRAFELEFT]++; break;
		case Qt::Key_D: holdingKeys[HOLDING_STRAFERIGHT]++; break;
		case Qt::Key_Left: holdingKeys[HOLDING_TURNLEFT]++; break;
		case Qt::Key_Right: holdingKeys[HOLDING_TURNRIGHT]++; break;

		case Qt::Key_Z:
		case Qt::Key_X:
		if (mode == MODE_EDITING)
		{
			if (event->key() == Qt::Key_X)
				editingScaleFactor *= 1.1;
			else if (editingScaleFactor > 1.0)
				editingScaleFactor /= 1.1;
			_SetupProjectionMatrix();
			_SetupModelMatrix();
			update();
			emit(zoomChanged());
		}
		break;


		default:
		QGLWidget::keyPressEvent(event);
	}
}

void MazeWidget3D::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Up:
		case Qt::Key_W: holdingKeys[HOLDING_FORWARD]--; break;
		case Qt::Key_Down:
		case Qt::Key_S: holdingKeys[HOLDING_BACKWARD]--; break;
		case Qt::Key_A: holdingKeys[HOLDING_STRAFELEFT]--; break;
		case Qt::Key_D: holdingKeys[HOLDING_STRAFERIGHT]--; break;
		case Qt::Key_Left: holdingKeys[HOLDING_TURNLEFT]--; break;
		case Qt::Key_Right: holdingKeys[HOLDING_TURNRIGHT]--; break;

		default:
		QGLWidget::keyReleaseEvent(event);
	}
}

#ifndef GL_LIGHT_MODEL_COLOR_CONTROL
#define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#endif

#ifndef GL_SEPARATE_SPECULAR_COLOR
#define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#endif

void MazeWidget3D::initializeGL()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	// glClearColor(0.15, 0.15, 0.15, 0.0);
	// glClearColor(0.0, 0.0, 0.0, 0.0);
	// glClearColor(1.0, 1.0, 1.0, 0.0);
	glClearColor(245/255.0, 245/255.0, 220/255.0, 0.0);
	glEnable(GL_CULL_FACE);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// glShadeModel(GL_SMOOTH);
	// glDisable(GL_LINE_SMOOTH);

	/*glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glColorMaterial(GL_FRONT, GL_SPECULAR);
	glEnable(GL_COLOR_MATERIAL);
	// glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	// glColorMaterial(GL_FRONT, GL_EMISSION);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	// glEnable(GL_LIGHTING);
	{
		static const GLfloat white[] = {0.8f, 0.8f, 0.8f, 1.0f};
		static const GLfloat cyan[] = {0.f, .8f, .8f, 1.f};
		glMaterialfv(GL_FRONT, GL_DIFFUSE, cyan);
		glMaterialfv(GL_FRONT, GL_SPECULAR, white);
		static const GLfloat shininess[] = {50};
		glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	}*/
	// glEnable(GL_RESCALE_NORMAL);

	// displayListMaze = glGenLists(1);

	maze.setContext(this);
}

void MazeWidget3D::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);

	_SetupModelMatrix();
	_SetupProjectionMatrix();
}

void MazeWidget3D::paintGL()
{
	/*static GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	static GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	// static GLfloat LightPosition[] = { 0.0f, 0.0f, -100.0f, 1.0f };
	// static GLfloat LightPosition[] = { 0.0f, 0.0f, -1000.0f, 1.0f };
	static GLfloat LightPosition[] = { 1000.0f, 300.0f, -125.0f, 1.0f };
	// static GLfloat LightPosition[] = {1.0, 0.5, 1.0, 0.0};
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT1);*/

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (won && mode == MODE_MOUSELOOK)
		return;
	// if (!(maze == oldMaze))
	{
		// oldMaze = maze;
		// glNewList(displayListMaze, GL_COMPILE);
		maze.drawGrid(); // if (!playing)
		maze.draw(mode == MODE_EDITING);
		// glEndList();
	}

	for (QVector<Path>::const_iterator it = paths.begin(); it != paths.end(); ++it)
		it->draw();

	// glCallList(displayListMaze);
	// if (mode == MODE_EDITING)
	{
		glLineWidth(4.0);
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < drawPoints.size(); i++)
			glVertex3i(drawPoints[i].x(), drawPoints[i].y(), -5.0);
		glEnd();
		glColor3f(1.0, 1.0, 1.0);
	}

	// logger.draw();

	_DrawCamera();
}

float MazeWidget3D::calculateAspectRatio() const
{
	return static_cast<GLfloat>(width())/static_cast<GLfloat>(height());
}

void MazeWidget3D::_SetupProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	const GLfloat aspectRatio = calculateAspectRatio();

	if (mode == MODE_MOUSELOOK || mode == MODE_OVERVIEW)
	{
		gluPerspective(45.0, aspectRatio, 10, 100000.0);
	}
	else if (mode == MODE_EDITING)
	{
		// const GLfloat mazeAspectRatio = maze.getWidth()/maze.getHeight();

		// const GLfloat ratio = aspectRatio/mazeAspectRatio;

		// const GLfloat widthRatio = std::max(static_cast<GLfloat>(1.0), aspectRatio)/std::max(static_cast<GLfloat>(1.0), mazeAspectRatio);
		// const GLfloat heightRatio = std::max(static_cast<GLfloat>(1.0), static_cast<GLfloat>(1.0)/aspectRatio)/std::max(static_cast<GLfloat>(1.0), static_cast<GLfloat>(1.0)/mazeAspectRatio);

		// const GLfloat widthRatio = std::max(static_cast<GLfloat>(1.0), aspectRatio);
		// const GLfloat heightRatio = std::max(static_cast<GLfloat>(1.0), static_cast<GLfloat>(1.0)/(aspectRatio));

		// const GLfloat mazeWidth = std::max(maze.getWidth(), maze.getHeight());
		// const GLfloat mazeHeight = mazeWidth;

		const GLfloat viewWidth = width();
		const GLfloat viewHeight = height();

		glOrtho(-viewWidth*editingScaleFactor,
				viewWidth*editingScaleFactor,
				viewHeight*editingScaleFactor,
				-viewHeight*editingScaleFactor,
				100000,
				-100000);
		// glOrtho(-widthRatio*mazeWidth*(editingScaleFactor - 1.0), widthRatio*mazeWidth*editingScaleFactor, heightRatio*mazeHeight*editingScaleFactor, -heightRatio*mazeHeight*(editingScaleFactor - 1.0), 100000, -100000);
	}
}

QSize MazeWidget3D::getMazeSize() const
{
	return QSize(maze.getWidth(), maze.getHeight());
}

int MazeWidget3D::getHorizontalPageStep() const
{
	return width()*editingScaleFactor;
}

int MazeWidget3D::getVerticalPageStep() const
{
	return height()*editingScaleFactor;
}

int MazeWidget3D::getVerticalOffset() const
{
	return (mode == MODE_OVERVIEW) ? overviewVerticalOffset : verticalOffset;
}

int MazeWidget3D::getHorizontalOffset() const
{
	return (mode == MODE_OVERVIEW) ? overviewHorizontalOffset : horizontalOffset;
}

void MazeWidget3D::slot_SetVerticalOffset(int x)
{
	// const bool noticableDifference = static_cast<float>(abs(x - verticalOffset)) >= editingScaleFactor;
	((mode == MODE_OVERVIEW) ? overviewVerticalOffset : verticalOffset) = x;
	// if (noticableDifference)
	{
		_SetupModelMatrix();
		update();
	}
}

void MazeWidget3D::slot_SetHorizontalOffset(int x)
{
	// const bool noticableDifference = static_cast<float>(abs(x - horizontalOffset)) >= editingScaleFactor;
	((mode == MODE_OVERVIEW) ? overviewHorizontalOffset : horizontalOffset) = x;
	// if (noticableDifference)
	{
		_SetupModelMatrix();
		update();
	}
}

void MazeWidget3D::slot_SetJoystickDeadZone(double deadZone)
{
	joystickDeadZone = deadZone;
}

void MazeWidget3D::slot_SetJoystickWalkingSpeed(double walkingSpeed)
{
	joystickWalkingSpeed = walkingSpeed;
}

void MazeWidget3D::slot_SetJoystickTurningSpeed(double turningSpeed)
{
	joystickTurningSpeed = turningSpeed;
}
// #include <QDebug>
void MazeWidget3D::slot_SetCurrentWallTexture(const QString &filename)
{
	// qDebug() << "MazeWidget3D::slot_SetCurrentWallTexture: " << filename;
	currentWallTextureFilename = filename;
}

void MazeWidget3D::slot_SetCurrentWallHeight(int newHeight)
{
	currentWallHeight = qBound(0, newHeight, 10);
}

void MazeWidget3D::_SetupModelMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (mode == MODE_MOUSELOOK)
	{
		camera.setupMatrices();
	}
	else if (mode == MODE_EDITING)
	{
		glTranslatef(-horizontalOffset, -verticalOffset, 0.0);
	}
	else if (mode == MODE_OVERVIEW)
	{
		const QVector3D mapCenter = QVector3D(overviewHorizontalOffset, overviewVerticalOffset, -GRID_SIZE/2);// + tumble.position;
		const QVector3D viewFrom = mapCenter - tumble.view*(maze.getWidth() > maze.getHeight() ? maze.getWidth() : maze.getHeight())*1.3*scaleFactor;
		const QVector3D up = QVector3D(0.0, -1.0, 0.0);

		Camera outerSpaceView(viewFrom, (mapCenter - viewFrom).normalized(), up);
		outerSpaceView.setupMatrices();
	}
}

void MazeWidget3D::_DrawCamera()
{
	// draw a player radius indicator
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		glTranslatef(camera.position.x(), camera.position.y(), camera.position.z());
		glRotatef(180.0, 1.0, 0.0, 0.0);
		glColor3f(0.0, 0.0, 1.0);
		gluDisk(quadric, 95.0, 100.0, 32, 32);
		glColor3f(1.0, 1.0, 1.0);

		glPopMatrix();
	}

	// draw an camera indicator
	if (mode != MODE_MOUSELOOK)
		camera.draw();
}
