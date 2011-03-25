#ifndef POSITIONLOGGER_H
#define POSITIONLOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QVector3D>
#include <QVector>
#include <QElapsedTimer>

class PositionLogger
{
public:
	PositionLogger();
	~PositionLogger();
	void start(const QString &filename, const QString &subjectName, const QString &mazeFileName);
	void log(float x, float y, float angle);
	void end();
	void draw() const;
protected:
	qint64 lastTime;
	float old_x, old_y, lastUsed_x, lastUsed_y, old_angle;
	QFile outFile;
	QTextStream outStream;
	typedef QVector<QVector3D> QVector3DVector;
	QVector3DVector linePoints;
	QElapsedTimer timer;
};

#endif
