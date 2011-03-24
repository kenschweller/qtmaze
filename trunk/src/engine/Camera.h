#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>

class Camera
{
public:
	Camera();
	Camera(const QVector3D &p, const QVector3D &v, const QVector3D &u);
	
	QVector3D getForward() const;
	QVector3D getBackward() const;
	QVector3D getLeft() const;
	QVector3D getRight() const;
	
	void moveView(float dx, float dy, bool freeLook = false);
	void setupMatrices();
	
	QVector3D position;
	QVector3D view;
	QVector3D up;
};

#endif
