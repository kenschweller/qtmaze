#ifndef IMAGEPANE_H
#define IMAGEPANE_H

#include <QDockWidget>
class QListWidget;
class QListWidgetItem;

class ImagePane : public QDockWidget
{
	Q_OBJECT
public:
	ImagePane(QWidget *parent = NULL);
	QListWidget * landmarks() {return _landmarks;}
	QListWidget * textures() {return _textures;}
signals:
	void wallTextureChanged(const QString &filename);
protected slots:
	void slot_CurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
protected:
	QListWidget *_landmarks;
	QListWidget *_textures;
};

#endif
