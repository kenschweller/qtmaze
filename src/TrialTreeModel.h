#ifndef TRIALTREEMODEL_H
#define TRIALTREEMODEL_H

#include <QStandardItemModel>

class TrialTreeModel : public QStandardItemModel
{
	Q_OBJECT
public:
	TrialTreeModel(QObject *parent = NULL);
	~TrialTreeModel();
	QStringList mimeTypes() const;

	void appendMazeFile(const QString &filename);
protected:
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
};

#endif
