#include "ImagePane.h"

#include <QTabWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDir>

static void PopulateWithImages(QListWidget * const imagelist, const QString &path)
{
	QDir imagesDir(path);
	QStringList extensions;
	extensions << "*.jpg" << "*.png" << "*.bmp" << "*.gif";
	const QFileInfoList fileList = imagesDir.entryInfoList(extensions, QDir::Files, QDir::Name | QDir::IgnoreCase);
	for (QFileInfoList::const_iterator it = fileList.begin(); it != fileList.end(); ++it)
	{
		QListWidgetItem * const item = new QListWidgetItem(QIcon(QPixmap(it->absoluteFilePath())), it->fileName());
		imagelist->addItem(item);
	}
}

ImagePane::ImagePane(QWidget *parent) : QDockWidget("Image Picker", parent), _landmarks(NULL), _textures(NULL)
{
	_landmarks = new QListWidget;
	_landmarks->setViewMode(QListView::IconMode);
	_landmarks->setIconSize(QSize(96, 96));
	_landmarks->setDragEnabled(true);
	_landmarks->setAcceptDrops(false);

	_textures = new QListWidget;
	_textures->setViewMode(QListView::IconMode);
	_textures->setIconSize(QSize(192, 192));
	_textures->setDragEnabled(true);
	_textures->setAcceptDrops(false);

	PopulateWithImages(_landmarks, "data/landmarks/");
	PopulateWithImages(_textures, "data/textures/");

	QTabWidget * const tabs = new QTabWidget(this);
	tabs->addTab(_landmarks, "Landmarks");
	tabs->addTab(_textures, "Wall Textures");

	setWidget(tabs);
	setFeatures(QDockWidget::DockWidgetClosable);

	connect(_textures, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(slot_CurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));
}
// #include <QDebug>
void ImagePane::slot_CurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	// qDebug() << "ImagePane::slot_CurrentItemChanged: Current item changed to " << current->text();
	emit(wallTextureChanged(current->text()));
}
