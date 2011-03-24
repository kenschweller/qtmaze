#ifndef TRIALTREEVIEW_H
#define TRIALTREEVIEW_H

#include <QTreeView>

class TrialTreeView : public QTreeView
{
	Q_OBJECT
public:
	TrialTreeView(QWidget *parent = NULL);
	~TrialTreeView();
public slots:
	void slot_SetEnabled(bool enabled);
protected:
	// void dragEnterEvent(QDragEnterEvent *event);
	// void dragLeaveEvent(QDragLeaveEvent *event);
	// void dragMoveEvent(QDragMoveEvent *event);
	// void dropEvent(QDropEvent *event);
};

#endif
