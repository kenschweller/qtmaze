#ifndef TRIALPANE_H
#define TRIALPANE_H

#include <QDockWidget>
#include <QModelIndex>
class TrialTreeModel;
class QPushButton;

class TrialPane : public QDockWidget
{
	Q_OBJECT
public:
	TrialPane(QWidget *parent = NULL);

	void restart();
	QString getNextMaze();
signals:
	void canRunTrials(bool enabled);
protected slots:
	void slot_New();
	void slot_Open();
	void slot_Save();
	void slot_SaveAs();

	void slot_ModelChanged();
protected:
	void _Save(const QString &filename);
	TrialTreeModel *_model;
	QModelIndex _nextMaze;
	QString _listFilename;
	QPushButton *saveButton;
};

#endif
