#ifndef TRIALPANE_H
#define TRIALPANE_H

#include <QDockWidget>
class QStandardItemModel;

class TrialPane : public QDockWidget
{
public:
	TrialPane(QWidget *parent = NULL);
protected:
	QStandardItemModel *_model;
};

#endif
