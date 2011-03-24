#ifndef TRIALTREEMODEL_H
#define TRIALTREEMODEL_H

#include <QStandardItemModel>

class TrialTreeModel : public QStandardItemModel
{
	Q_OBJECT
public:
	TrialTreeModel(QObject *parent = NULL);
	~TrialTreeModel();
	Qt::DropActions supportedDragActions() const;
	QStringList mimeTypes() const;
protected:
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
};

#endif
