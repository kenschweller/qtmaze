#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QScopedPointer>
#include <QGLWidget>
#include <memory>

class Camera
{
public:
	Camera();
	Camera(const QVector3D &p, const QVector3D &v, const QVector3D &u);
	~Camera();

	QVector3D getForward() const;
	QVector3D getBackward() const;
	QVector3D getLeft() const;
	QVector3D getRight() const;

	void moveView(float dx, float dy, bool freeLook = false);
	void setupMatrices();
	void draw() const;

	QVector3D position;
	QVector3D view;
	QVector3D up;

	struct GLUquadricDeleter
	{
		static inline void cleanup(GLUquadric *pointer)
		{
			gluDeleteQuadric(pointer);
		}
	};
	static QScopedPointer<GLUquadric, GLUquadricDeleter> quadric;
};

#endif
