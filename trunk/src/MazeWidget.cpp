#include "MazeWidget.h"
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

QPoint MazeWidget3D::_Unproject(const QPoint &point)
{
	GLdouble modelviewMatrix[16];
	GLdouble projectionMatrix[16];
	GLint viewport[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble x, y, z;
	GLdouble x2, y2, z2;
	gluUnProject(point.x(), height() - point.y(), 0.0, modelviewMatrix, projectionMatrix, viewport, &x, &y, &z);
	gluUnProject(point.x(), height() - point.y(), 1.0, modelviewMatrix, projectionMatrix, viewport, &x2, &y2, &z2);

	QVector3D rayEnd(x, y, z);
	/*QVector3D rayStart(x2, y2, z2);

	const QVector3D mapCenter = QVector3D(maze.getWidth()/2, maze.getHeight()/2, -GRID_SIZE/2);// + tumble.position;
	QVector3D viewFrom = mapCenter - tumble.view*(maze.getWidth() > maze.getHeight() ? maze.getWidth() : maze.getHeight())*1.3*scaleFactor;

	if (mode == MODE_OVERVIEW)
	{
		rayEnd = viewFrom - (rayEnd - viewFrom).normalized()*viewFrom.z();
	}
	else if (mode == MODE_MOUSELOOK)
	{
		// rayEnd -= camera.position;
		rayEnd = camera.position - camera.view*camera.position.z();
	}*/

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
	joystickDeadZone(0.05), joystickWalkingSpeed(1.0), joystickTurningSpeed(1.0)
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

void MazeWidget3D::reset(int width, int height, const QString description)
{
	_filename = "";
	maze.reset(width, height);
	// maze.setDescription(description); // TODO
	restart(false);
}

void MazeWidget3D::_RestartLogging()
{
	logger.end();
	if (isParticipating())
	{
		const QString basePath("logs/" + _participantName + "/" + QFileInfo(_filename).completeBaseName());
		QDir::current().mkpath(basePath);
		QString filename;
		for (int i = 0; i < 9999; i++)
		{
			filename = basePath + "/" + QString::number(i).rightJustified(4, '0') + ".txt";
			if (!QFile::exists(filename))
				break;
		}
		logger.start(filename);
	}
}

bool MazeWidget3D::open(const QString &filename, bool pplaying)
{
	const bool result = maze.load(filename);
	if (result)
	{
		_filename = filename;
		restart(pplaying);
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
		updateGL();
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
		updateGL();
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
		updateGL();
		setMouseTracking(false);
	}
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
	updateGL();
	emit(zoomChanged());
}

class SleeperThread : public QThread
{
public:
	static void Sleep(int seconds) {sleep(seconds);}
};

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
	updateGL();

	if (playing && maze.mapPointInGoalRadius(camera.position.toPoint()))
	{
		goalSound.play();
		playing = false;
		won = true;
		logger.end();
		updateGL();
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
		updateGL();
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
			updateGL();
		}
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-newgoal"))
	{
		event->acceptProposedAction();
		maze.startDroppingGoal();
		maze.moveDroppingGoal(mapPoint);
		updateGL();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-cameraposition"))
	{
		event->acceptProposedAction();
	}
}

void MazeWidget3D::dragLeaveEvent(QDragLeaveEvent *event)
{
	maze.endDroppingImage(false);
	maze.endDroppingGoal(false);
	updateGL();
}

void MazeWidget3D::dragMoveEvent(QDragMoveEvent *event)
{
	const QPoint mapPoint = _Unproject(event->pos());
	if (event->mimeData()->hasFormat("application/x-qtmaze-startingorientation"))
	{
		const Orientation orientation = maze.getNearestOrientation(mapPoint);
		maze.setStartingOrientation(orientation);
		updateGL();
		return;
	}
	else if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		// printf("Drag moving to (%i, %i)\n", event->pos().x(), event->pos().y());
		maze.moveDroppingImage(mapPoint);
		updateGL();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-cameraposition"))
	{
		camera.position.setX(mapPoint.x());
		camera.position.setY(mapPoint.y());
		updateGL();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-newgoal"))
	{
		maze.moveDroppingGoal(mapPoint);
		updateGL();
	}
}

void MazeWidget3D::dropEvent(QDropEvent *event)
{
	const QPoint mapPoint = _Unproject(event->pos());
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		maze.endDroppingImage(true);
		updateGL();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-startingorientation"))
	{
		const Orientation orientation = maze.getNearestOrientation(mapPoint);
		maze.setStartingOrientation(orientation);
		updateGL();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-newgoal"))
	{
		maze.moveDroppingGoal(mapPoint);
		maze.endDroppingGoal(true);
		updateGL();
	}
	else if (event->mimeData()->hasFormat("application/x-qtmaze-cameraposition"))
	{
		camera.position.setX(mapPoint.x());
		camera.position.setY(mapPoint.y());
		updateGL();
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
		updateGL();
	}
	else if (selected == startAction)
	{
		maze.setStartingOrientation(orientation);
		updateGL();
	}
	else if (selected == placeAction)
	{
		camera.position.setX(mapPoint.x());
		camera.position.setY(mapPoint.y());
		updateGL();
	}
}

void MazeWidget3D::mousePressEvent(QMouseEvent *event)
{
	if (occupation != OCCUPATION_NONE)
		return;

	lastPos = event->pos();
	if (mode == MODE_EDITING)
	{
		if (event->button() == Qt::LeftButton)
		{
			const QPoint mapPoint = _Unproject(event->pos());

			const Orientation orientation = maze.getNearestOrientation(mapPoint);
			const QString filename = maze.getPicture(orientation).filename;
			if (QLineF(camera.position.toPoint(), mapPoint).length() < CAMERA_RADIUS)
			{
				QDrag * const drag = new QDrag(this);
				{
					QMimeData * const mimeData = new QMimeData;
					mimeData->setData("application/x-qtmaze-cameraposition", QByteArray());
					drag->setMimeData(mimeData);
				}
				drag->start(Qt::CopyAction | Qt::MoveAction);
				delete drag;
			}
			else if (filename.size() > 0)
			{
				maze.removePicture(orientation);
				QDrag * const drag = new QDrag(this);

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
						drag->setMimeData(mimeData);
					}
				}

				drag->start(Qt::CopyAction | Qt::MoveAction);

				delete drag;
			}
			else if (maze.mapPointInGoalRadius(mapPoint))
			{
				maze.removeGoal(orientation.tile);
				QDrag * const drag = new QDrag(this);
				{
					QMimeData * const mimeData = new QMimeData;
					mimeData->setData("application/x-qtmaze-newgoal", QByteArray());
					drag->setMimeData(mimeData);
				}
				drag->start(Qt::CopyAction | Qt::MoveAction);
				delete drag;
			}
			else if (orientation == maze.getStartingOrientation())
			{
				QDrag * const drag = new QDrag(this);
				{
					QMimeData * const mimeData = new QMimeData;
					mimeData->setData("application/x-qtmaze-startingorientation", QByteArray());
					drag->setMimeData(mimeData);
				}
				drag->start(Qt::CopyAction | Qt::MoveAction);
				delete drag;
			}
			else if (event->modifiers() & Qt::AltModifier)
			{
				// if (abs(mapPoint.x()) < RESIZE_THRESHOLD)
			}
			else
			{
				occupation = OCCUPATION_DRAWING;
				drawPoints.clear();
				drawPoints.append(mapPoint);
				if (event->modifiers() & Qt::ShiftModifier)
					lastTilePos = maze.getNearestTile(mapPoint);
				else
					lastTilePos = maze.getNearestVertex(mapPoint);
			}
		}
	}
}

void MazeWidget3D::mouseReleaseEvent(QMouseEvent *event)
{
	lastPos = event->pos();
	// if (mode == MODE_EDITING)
	{
		if (event->button() == Qt::LeftButton && occupation == OCCUPATION_DRAWING)
		{
			drawPoints.clear();
			updateGL();
			occupation = OCCUPATION_NONE;
		}
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

	if (mode == MODE_MOUSELOOK || mode == MODE_OVERVIEW)
	{
		if (mode == MODE_MOUSELOOK)
			camera.moveView(-dx, -dy, false);
		else
			tumble.moveView(dx*2, dy*2, true);
		_SetupModelMatrix();
		updateGL();
	}
	else if (mode == MODE_EDITING)
	{
		if (occupation == OCCUPATION_DRAWING)
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
			updateGL();
		}
	}
}

void MazeWidget3D::mouseDoubleClickEvent(QMouseEvent *event)
{
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
	updateGL();
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
			updateGL();
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
			updateGL();
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
			updateGL();
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

	/*glShadeModel(GL_SMOOTH);
	glDisable(GL_LINE_SMOOTH);*/

	/*glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);*/
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
	static GLfloat LightPosition[] = { 0.0f, 0.0f, -100.0f, 1.0f };
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
		maze.draw();
		// glEndList();
	}
	// glCallList(displayListMaze);
	// if (mode == MODE_EDITING)
	{
		glLineWidth(4.0);
		glColor3f(1.0, 0, 0);
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
		updateGL();
	}
}

void MazeWidget3D::slot_SetHorizontalOffset(int x)
{
	// const bool noticableDifference = static_cast<float>(abs(x - horizontalOffset)) >= editingScaleFactor;
	((mode == MODE_OVERVIEW) ? overviewHorizontalOffset : horizontalOffset) = x;
	// if (noticableDifference)
	{
		_SetupModelMatrix();
		updateGL();
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

#include <algorithm>
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
		// const GLfloat mazeAspectRatio = maze.getWidth()/maze.getHeight();
		// const GLfloat aspectRatio = calculateAspectRatio();
		// const GLfloat mazeSize = std::min(maze.getWidth(), maze.getHeight());

		glTranslatef(-horizontalOffset, -verticalOffset, 0.0);

		/*if (aspectRatio > 1.0)
			glTranslatef((aspectRatio - 1.0)*mazeSize/2.0, 0.0, 0.0);
		else
			glTranslatef(0.0, (1.0/aspectRatio - 1.0)*mazeSize/2.0, 0.0);

		if (mazeAspectRatio < 1.0)
			glTranslatef(0.0, -(maze.getHeight()-maze.getWidth())/2.0, 0.0);
		else
			glTranslatef(-(maze.getWidth()-maze.getHeight())/2.0, 0.0, 0.0);*/
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
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		glTranslatef(camera.position.x(), camera.position.y(), camera.position.z());
		const QLineF viewLine(0.0, 0.0, camera.view.x(), camera.view.y());
		const QLineF viewLine2(0.0, 0.0, -camera.view.z(), viewLine.length());
		glRotatef(viewLine.angle() - 90.0, 0.0, 0.0, -1.0);
		glRotatef(viewLine2.angle(), 1.0, 0.0, 0.0);
		glRotatef(180.0, 1.0, 0.0, 0.0);
		glColor3f(0.0, 0.0, 1.0);
		gluCylinder(quadric, 50.0, 1.0, 50.0, 32, 1);
		glPushMatrix();
		glRotatef(180.0, 1.0, 0.0, 0.0);
		gluDisk(quadric, 0.0, 50.0, 32, 32);
		glColor3f(0.0, 0.0, 0.0);
		gluDisk(quadric, 50.0, 52.0, 32, 1);
		glTranslatef(0.0, 0.0, 1.0);
		gluDisk(quadric, 10.0, 12.0, 32, 1);
		glColor3f(0.0, 0.0, 1.0);
		glPopMatrix();
		glTranslatef(0.0, 0.0, -50.0);
		gluCylinder(quadric, 10.0, 10.0, 50.0, 32, 1);
		glTranslatef(0.0, 0.0, -10.0);
		gluSphere(quadric, 20.0, 16, 16);
		glColor3f(1.0, 1.0, 1.0);

		glPopMatrix();
	}
}
