#ifndef IMAGEPANE_H
#define IMAGEPANE_H

#include <QDockWidget>
class QListWidget;

class ImagePane : public QDockWidget
{
public:
	ImagePane(QWidget *parent = NULL);
	QListWidget * imageList() {return _imageList;}
protected:
	QListWidget *_imageList;
};

#endif
