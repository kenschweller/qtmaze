#include "Path.h"
#include "defines.h"

#include <QFile>
#include <QMessageBox>
#include <QtOpenGL>

Path::Path()
{
}

Path::~Path()
{
}

void Path::load(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox msg;
		msg.setText("Error opening data file '" + filename + "'");
		msg.exec();
		return;
	}

	datapoints.clear();
	sampled.clear();

	while (!file.atEnd())
	{
		const QString line = file.readLine().trimmed();
		if (line.startsWith('#'))
			continue;
		const QStringList fields = line.split('\t');
		if (fields.size() < 4)
			continue;
		bool millisecondsOk = false, xOk = false, yOk = false, angleOk = false;
		const qint64 milliseconds = fields[0].toLongLong(&millisecondsOk);
		const float x             = fields[1].toFloat(&xOk);
		const float y             = fields[2].toFloat(&yOk);
		const float angle         = fields[3].toFloat(&angleOk);

		if (millisecondsOk && xOk && yOk && angleOk)
			addPoint(milliseconds, QPointF(x*GRID_SIZE, y*GRID_SIZE), angle);
	}
}

void Path::addPoint(qint64 milliseconds, const QPointF &position, float angle)
{
	static const float LINE_LENGTH = 20.0;

	const DataPoint datapoint(milliseconds, position, angle);
	datapoints.append(datapoint);
	if (sampled.empty() || QLineF(sampled.first().position, datapoints.last().position).length() >= LINE_LENGTH)
		sampled.append(datapoint);
}

void Path::draw() const
{
	glLineWidth(1.0);
	glColor3f(1.0, 1.0, 1.0);
	// glColor3f(0.0, 0.0, 1.0);
	const int evenNumber = (sampled.size() & ~1);
	glBegin(GL_LINES);
	for (int i = 0; i < evenNumber; i++)
		glVertex3f(sampled[i].position.x(), sampled[i].position.y(), -0.6 - 0.001*static_cast<float>(i));

	// glColor3f(0.5, 0.5, 0.5);
	// glColor3f(0.75, 0.75, 0.75);
	glColor3f(0.0, 0.0, 0.0);
	// glColor3f(0.0, 1.0, 0.0);
	const int oddNumber = ((sampled.size() - 1) & ~1) + 1;
	for (int i = 1; i < oddNumber; i++)
		glVertex3f(sampled[i].position.x(), sampled[i].position.y(), -0.6 - 0.001*static_cast<float>(i));
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
}
