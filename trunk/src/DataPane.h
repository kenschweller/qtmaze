#ifndef DATAPANE_H
#define DATAPANE_H

#include <QDockWidget>
class QTableView;
class QModelIndex;
class DataModel;

class DataPane : public QDockWidget
{
	Q_OBJECT
public:
	DataPane(QWidget *parent = NULL);
	~DataPane();
signals:
	void logActivated(const QString &logFilename, const QString &mazeFilename);
protected slots:
	void slot_Activated(const QModelIndex &index);
protected:
	QTableView *_tableView;
	DataModel *_model;
};

#endif
