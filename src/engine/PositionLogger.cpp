#include "PositionLogger.h"
#include "defines.h"

#include <QDateTime>
#include <QMessageBox>
#include <QGLWidget>
#include <limits>

PositionLogger::PositionLogger() :
	lastTime(-1),
	old_x(std::numeric_limits<float>::infinity()),
	old_y(std::numeric_limits<float>::infinity()),
	old_angle(std::numeric_limits<float>::infinity())
{
}

PositionLogger::~PositionLogger()
{
	end();
}

void PositionLogger::start(const QString &filename, const QString &subjectName, const QString &mazeFileName)
{
	outFile.setFileName(filename);
	if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		QMessageBox msg;
		msg.setText("Error opening opening log file '" + filename + "' for writing!");
		msg.exec();
		return;
	}
#if QT_VERSION >= 0x050000
	printf("Started logging to file '%s'...\n", filename.toLatin1().data());
#else
	printf("Started logging to file '%s'...\n", filename.toAscii().data());
#endif
	outStream.setDevice(&outFile);
	outStream << "# Maze: \"" << mazeFileName << "\"\n";
	outStream << "# Subject: \"" << subjectName << "\"\n";
	outStream << "# Date: " << QDateTime::currentDateTime().toString() << "\n";
	outStream << "ms\tX\tY\tAngle\n";
	timer.start();
	lastTime = -1;
	old_x = std::numeric_limits<float>::infinity();
	old_y = std::numeric_limits<float>::infinity();
	old_angle = std::numeric_limits<float>::infinity();
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
}

void PositionLogger::end()
{
	if (outFile.isOpen())
	{
#if QT_VERSION >= 0x050000
		printf("...Stopped logging to '%s'!\n", outFile.fileName().toLatin1().data());
#else
		printf("...Stopped logging to '%s'!\n", outFile.fileName().toAscii().data());
#endif
		outStream.setDevice(NULL);
		outFile.close();
	}
}
