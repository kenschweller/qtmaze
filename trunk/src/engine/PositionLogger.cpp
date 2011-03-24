#include "PositionLogger.h"
#include "defines.h"

#include <QMessageBox>
#include <QGLWidget>
#include <limits>

PositionLogger::PositionLogger() :
	lastTime(-1),
	old_x(std::numeric_limits<float>::infinity()),
	old_y(std::numeric_limits<float>::infinity()),
	lastUsed_x(std::numeric_limits<float>::infinity()),
	lastUsed_y(std::numeric_limits<float>::infinity()),
	old_angle(std::numeric_limits<float>::infinity())
{
}

PositionLogger::~PositionLogger()
{
	end();
}

void PositionLogger::start(const QString &filename)
{
	outFile.setFileName(filename);
	if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		QMessageBox msg;
		msg.setText("Error opening opening log file '" + filename + "' for writing!");
		msg.exec();
		return;
	}
	printf("Started logging to file '%s'...\n", filename.toAscii().data());
	outStream.setDevice(&outFile);
	outStream << "ms\tX\tY\tAngle\n";
	timer.start();
}

void PositionLogger::log(float x, float y, float angle)
{
	if (x == old_x && y == old_y && angle == old_angle)
		return;
	
	old_x = x;
	old_y = y;
	old_angle = angle;
	
	const qint64 newTime = timer.elapsed();
	if (outFile.isOpen() && newTime != lastTime)
	{
		outStream << newTime << "\t" << (x/GRID_SIZE) << "\t" << (y/GRID_SIZE) << "\t" << angle << "\n";
	}
	
	{
		const float dx = x - lastUsed_x;
		const float dy = y - lastUsed_y;
		const float distanceSquared = dx*dx + dy*dy;
		static const float LINE_LENGTH = 20;
		static const float LINE_LENGTH_SQUARED = LINE_LENGTH*LINE_LENGTH;
		
		if (distanceSquared >= LINE_LENGTH_SQUARED)
		{
			linePoints.append(QVector3D(x, y, -2.0));
			lastUsed_x = x;
			lastUsed_y = y;
		}
	}
}

void PositionLogger::end()
{
	if (outFile.isOpen())
	{
		printf("...Stopped logging!\n");
		outStream.setDevice(NULL);
		outFile.close();
	}
}

void PositionLogger::draw() const
{
	glLineWidth(4.0);
	glColor3f(1.0, 1.0, 1.0);
	const int evenNumber = (linePoints.size() & ~1);
	glBegin(GL_LINES);
	for (int i = 0; i < evenNumber; i++)
	{
		glVertex3f(linePoints[i].x(), linePoints[i].y(), linePoints[i].z() - 0.001*static_cast<float>(i));
	}
	
	glColor3f(0.5, 0.5, 0.5);
	const int oddNumber = ((linePoints.size() - 1) & ~1) + 1;
	for (int i = 1; i < oddNumber; i++)
	{
		glVertex3f(linePoints[i].x(), linePoints[i].y(), linePoints[i].z() - 0.001*static_cast<float>(i));
	}
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
}
