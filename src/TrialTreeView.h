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
	void contextMenuEvent(QContextMenuEvent *event);
};

#endif
