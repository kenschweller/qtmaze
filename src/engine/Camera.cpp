#include "Camera.h"

// #include <QtOpenGL>
#ifdef Q_WS_MAC
#include <OpenGL/glu.h>
#else
#include <GL/glu.h> // NOTE: this may need changing when porting to other platforms
#endif
#include <QQuaternion>

Camera::Camera() : position(0.5*250.0, 0.5*250.0, -250.0/2.0), view(0.0, -1.0, 0.0), up(0.0, 0.0, -1.0)
{	
}

Camera::Camera(const QVector3D &p, const QVector3D &v, const QVector3D &u) : position(p), view(v), up(u)
{
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
