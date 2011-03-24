#include "ImagePane.h"

#include <QListWidget>
#include <QDir>

ImagePane::ImagePane(QWidget *parent) : QDockWidget("Image Picker", parent), _imageList(NULL)
{
	_imageList = new QListWidget(this);
	_imageList->setViewMode(QListView::IconMode);
	_imageList->setIconSize(QSize(96, 96));
	_imageList->setDragEnabled(true);
	_imageList->setAcceptDrops(false);
	
	setWidget(_imageList);
	setFeatures(QDockWidget::DockWidgetClosable);
	
	QDir imagesDir("data/landmarks/");
	QStringList extensions;
	extensions << "*.jpg" << "*.png" << "*.bmp" << "*.gif";
	const QFileInfoList fileList = imagesDir.entryInfoList(extensions, QDir::Files, QDir::Name | QDir::IgnoreCase);
	for (QFileInfoList::const_iterator it = fileList.begin(); it != fileList.end(); ++it)
	{
		QListWidgetItem * const item = new QListWidgetItem(QIcon(QPixmap(it->absoluteFilePath())), it->fileName());
		_imageList->addItem(item);
	}
}