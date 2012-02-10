#include "Camera.h"
#include "defines.h"

// #include <QtOpenGL>
// #ifdef Q_WS_MAC
// #include <OpenGL/glu.h>
// #else
// #include <GL/glu.h> // NOTE: this may need changing when porting to other platforms
// #endif
#include <QQuaternion>
#include <QGLWidget>

QScopedPointer<GLUquadric, Camera::GLUquadricDeleter> Camera::quadric;

Camera::Camera() : position(0.5*GRID_SIZE, 0.5*GRID_SIZE, -GRID_SIZE/2.0), view(0.0, -1.0, 0.0), up(0.0, 0.0, -1.0)
{
	if (quadric.isNull())
		quadric.reset(gluNewQuadric());
}

Camera::Camera(const QVector3D &p, const QVector3D &v, const QVector3D &u) : position(p), view(v), up(u)
{
	if (quadric.isNull())
		quadric.reset(gluNewQuadric());
}

Camera::~Camera()
{
	// gluDeleteQuadric(quadric);
}

QVector3D Camera::getForward() const
{
	QVector3D forward = view;
	forward.setZ(0);
	return forward.normalized();
}

QVector3D Camera::getBackward() const
{
	QVector3D backward = -view;
	backward.setZ(0);
	return backward.normalized();
}

QVector3D Camera::getLeft() const
{
	return QVector3D::crossProduct(getForward(), -up).normalized();
}

QVector3D Camera::getRight() const
{
	return QVector3D::crossProduct(getForward(), up).normalized();
}

void Camera::moveView(float dx, float dy, bool freeLook)
{
	const QQuaternion y_quat = QQuaternion::fromAxisAndAngle(freeLook ? QVector3D(1, 0, 0) : getRight(), -dy);
	const QQuaternion x_quat = QQuaternion::fromAxisAndAngle(up, -dx);
	view = y_quat.rotatedVector(x_quat.rotatedVector(view));
}

void Camera::setupMatrices()
{
	const QVector3D target = position + view;
	gluLookAt(
			position.x(), position.y(), position.z(),
			  target.x(),   target.y(),   target.z(),
			      up.x(),       up.y(),       up.z());
}

void Camera::draw() const
{
	const QLineF viewLine(0.0, 0.0, view.x(), view.y());
	const QLineF viewLine2(0.0, 0.0, -view.z(), viewLine.length());

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(position.x(), position.y(), position.z());
	glRotatef(viewLine.angle() - 90.0, 0.0, 0.0, -1.0);
	glRotatef(viewLine2.angle(), 1.0, 0.0, 0.0);
	glRotatef(180.0, 1.0, 0.0, 0.0);
	glColor3f(0.0, 0.0, 1.0);
	gluCylinder(quadric.data(), 50.0, 1.0, 50.0, 32, 1);
	glPushMatrix();
	glRotatef(180.0, 1.0, 0.0, 0.0);
	gluDisk(quadric.data(), 0.0, 50.0, 32, 32);
	glColor3f(0.0, 0.0, 0.0);
	gluDisk(quadric.data(), 50.0, 52.0, 32, 1);
	glTranslatef(0.0, 0.0, 1.0);
	gluDisk(quadric.data(), 10.0, 12.0, 32, 1);
	glColor3f(0.0, 0.0, 1.0);
	glPopMatrix();
	glTranslatef(0.0, 0.0, -50.0);
	gluCylinder(quadric.data(), 10.0, 10.0, 50.0, 32, 1);
	glTranslatef(0.0, 0.0, -10.0);
	gluSphere(quadric.data(), 20.0, 16, 16);
	glColor3f(1.0, 1.0, 1.0);

	glPopMatrix();
}