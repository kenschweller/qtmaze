#ifndef MAZEPANE_H
#define MAZEPANE_H

#include <QWidget>
class MazeWidget3D;
class QScrollBar;

class MazePane : public QWidget
{
	Q_OBJECT
public:
	MazePane(QWidget *parent = NULL);
	MazeWidget3D * getMazeWidget3d() {return _mazeWidget3d;}
	
	void showScrollbars();
	void hideScrollbars();
public slots:
	void slot_RefreshScrollbars();
protected:
	void resizeEvent(QResizeEvent *event);
	QScrollBar *_verticalScrollBar;
	QScrollBar *_horizontalScrollBar;
	MazeWidget3D *_mazeWidget3d;
};

#endif
