

	// NSWE
	/*const QLineF wallLines[] =
	{
		QLineF(QPointF(0.0, 0.0), QPointF(-GRID_SIZE, 0.0)), // north western corner, going westward
		QLineF(QPointF(0.0, 0.0), QPointF(0.0, -GRID_SIZE)), // north western corner, going northward
		QLineF(QPointF(GRID_SIZE, 0.0), QPointF(GRID_SIZE, -GRID_SIZE)), // north eastern corner, going northward
		QLineF(QPointF(GRID_SIZE, 0.0), QPointF(2*GRID_SIZE, 0.0)), // north eastern corner, going eastward
		QLineF(QPointF(GRID_SIZE, GRID_SIZE), QPointF(GRID_SIZE*2, GRID_SIZE)), // south eastern corner, going eastward
		QLineF(QPointF(GRID_SIZE, GRID_SIZE), QPointF(GRID_SIZE, GRID_SIZE*2)), // south eastern corner, going southward
		QLineF(QPointF(0.0, GRID_SIZE), QPointF(-GRID_SIZE, GRID_SIZE)), // south western corner, going westward
		QLineF(QPointF(0.0, GRID_SIZE), QPointF(0.0, 2*GRID_SIZE)) // south western corner, going southward
	};
	const bool wallLinesEnabled[] =
	{
		(vertexAt(tile) & WestWall) && !(vertexAt(tile) & SouthWall),
		(vertexAt(tile) & NorthWall) && !(vertexAt(tile) & EastWall),
		(vertexAt(tile + QPoint(1, 0)) & NorthWall) && !(vertexAt(tile + QPoint(1, 0)) & WestWall),
		(vertexAt(tile + QPoint(1, 0)) & EastWall) && !(vertexAt(tile + QPoint(1, 0)) & SouthWall),
		(vertexAt(tile + QPoint(1, 1)) & EastWall) && !(vertexAt(tile + QPoint(1, 1)) & NorthWall),
		(vertexAt(tile + QPoint(1, 1)) & SouthWall) && !(vertexAt(tile + QPoint(1, 1)) & WestWall),
		(vertexAt(tile + QPoint(0, 1)) & WestWall) && !(vertexAt(tile + QPoint(0, 1)) & NorthWall),
		(vertexAt(tile + QPoint(0, 1)) & SouthWall) && !(vertexAt(tile + QPoint(0, 1)) & EastWall)
	};*/

	/*const QPointF hitboxCorners[] =
	{
		QPointF(hitboxCenter + QPointF(-PLAYER_RADIUS, -PLAYER_RADIUS)),
		QPointF(hitboxCenter + QPointF(PLAYER_RADIUS, -PLAYER_RADIUS)),
		QPointF(hitboxCenter + QPointF(PLAYER_RADIUS, PLAYER_RADIUS)),
		QPointF(hitboxCenter + QPointF(-PLAYER_RADIUS, PLAYER_RADIUS))
	};
	const QLineF hitboxLines[] =
	{
		QLineF(hitboxCorners[0], hitboxCorners[0] + displacement),
		QLineF(hitboxCorners[1], hitboxCorners[1] + displacement),
		QLineF(hitboxCorners[2], hitboxCorners[2] + displacement),
		QLineF(hitboxCorners[3], hitboxCorners[3] + displacement)
	};
	
	for (unsigned int i = 0; i < ARRAYSIZE(hitboxLines); i++)
	{
		const QLineF hitboxLine = hitboxLines[i];
		for (unsigned int j = 0; j < ARRAYSIZE(wallLines); j++)
		{
			if (!wallLinesEnabled[j])
				continue;
			const QLineF &wallLine = wallLines[j];
			QPointF intersectionPoint;
			const QLineF::IntersectType type = wallLine.intersect(hitboxLine, &intersectionPoint);
			if (type != QLineF::BoundedIntersection)
				continue;
			const QLineF newDisplacement(hitboxLine.p1(), intersectionPoint);
			if (newDisplacement.length() < QLineF(QPointF(0.0, 0.0), displacement).length())
			{
				displacement = newDisplacement.p2() - newDisplacement.p1();
			}
		}
	}*/

	/*const float cornerDistances[] =
	{
		northWestDistance,
		northEastDistance,
		southEastDistance,
		southWestDistance
	};*/

	/*const QLineF dispLine = QLineF(QPointF(0.0, 0.0), displacement);
	const QLineF movementLine(position, position + dispLine.unitVector().p2()*PLAYER_RADIUS);
	// const float beamLength = movementLine.length();
	
	const QPointF northWestPoint(westWallDistance, northWallDistance);
	const QPointF northEastPoint(eastWallDistance, northWallDistance);
	const QPointF southEastPoint(eastWallDistance, southWallDistance);
	const QPointF southWestPoint(westWallDistance, southWallDistance);
	
	if (northWestCorner && QLineF(position + displacement, northWestPoint).length() < PLAYER_RADIUS)
	{
		// const QLineF northWestLine(position, northWestPoint);
		const QLineF westWall(northWestPoint, northWestPoint - QPointF(0, GRID_SIZE));
		// QPointF intersection;
		// const QLineF::IntersectType type = westWall.intersect(movementLine, &intersection);
		// if (type != QLineF::NoIntersection)
		// if (type == QLineF::BoundedIntersection)
		{
			QLineF tmp = dispLine;
			for (int i = 0; i < 360; i++)
			{
				tmp.setAngle(tmp.angle() - 1.0);
				if (QLineF(position + tmp.p2(), northWestPoint).length() >= PLAYER_RADIUS)
					return position + tmp.p2();
			}
		}
		// return position;
	}
	if (northEastCorner && QLineF(position + displacement, northEastPoint).length() < PLAYER_RADIUS)
	{
		// return position;
	}
	if (southEastCorner && QLineF(position + displacement, southEastPoint).length() < PLAYER_RADIUS)
	{
		// return position;
	}
	if (southWestCorner && QLineF(position + displacement, southWestPoint).length() < PLAYER_RADIUS)
	{
		// return position;
	}*/