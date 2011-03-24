#ifndef ORIENTATION_H
#define ORIENTATION_H

#include <QPoint>
#include <QString>

class Orientation
{
public:
	enum Direction
	{
		North,
		East,
		South,
		West
	};
public:
	Orientation() : tile(0, 0), direction(North) {}
	Orientation(const QPoint &t, Direction d) : tile(t), direction(d) {}
	
	bool operator<(const Orientation &other) const;
	bool operator==(const Orientation &other) const;
	bool operator!=(const Orientation &other) const;
	
	QString directionToString() const;
	static Direction directionFromString(const QString &name);
	
	void draw() const;
	
	QPoint tile;
	Direction direction;
};

#endif
