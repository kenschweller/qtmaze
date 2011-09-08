#ifndef PATH_H
#define PATH_H

#include <QVector>
#include <QPointF>
#include <QString>
class QFile;

class Path
{
public:
	Path();
	~Path();

	void load(const QString &filename);

	void addPoint(qint64 milliseconds, const QPointF &position, float angle);

	void draw() const;
protected:
	class DataPoint
	{
	public:
		DataPoint() : milliseconds(0), angle(0.0) {}
		DataPoint(qint64 mmilliseconds, const QPointF &pposition, float aangle) : milliseconds(mmilliseconds), position(pposition), angle(aangle) {}

		qint64 milliseconds;
		QPointF position;
		float angle;
	};
	typedef QVector<DataPoint> DataPoints;
	DataPoints datapoints;
	DataPoints sampled;
};

#endif
