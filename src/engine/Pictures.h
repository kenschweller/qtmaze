#ifndef PICTURES_H
#define PICTURES_H

#include "Orientation.h"

#include <QTextStream>
#include <QMap>
#include <QString>
class QGLWidget;

class Pictures
{
public:
	class Picture
	{
	public:
		Picture() {}
		Picture(const Orientation &o, const QString &f) : orientation(o), filename(f) {}
		bool operator==(const Picture &other) const;
		bool operator!=(const Picture &other) const;
		Orientation orientation;
		QString filename;
	};
public:
	Pictures();
	~Pictures();
	
	Pictures &operator=(const Pictures &other);
	bool operator==(const Pictures &other) const;
	bool operator!=(const Pictures &other) const;
	
	Picture at(const Orientation orientation) const;
	
	void add(const Orientation orientation, const QString &filename);
	void remove(const Orientation orientation);
	void clear();
	
	void draw() const;
	void setContext(QGLWidget *newContext);
protected:
	class TextureEntry
	{
	public:
		TextureEntry(unsigned int id = 0) : instanceCount(1), textureObject(id) {}
		int instanceCount;
		unsigned int textureObject;
	};
	void _DeleteTextures();
	void _DrawPicture(const Picture &picture) const;
	typedef QMap<Orientation, Picture> PictureMap;
	typedef QMap<QString, TextureEntry> TextureMap;
	PictureMap pictures;
	TextureMap textures;
	QGLWidget *context;
	
	friend QTextStream & operator<<(QTextStream &, const Pictures &);
};

QTextStream & operator<<(QTextStream &out, const Pictures &pictures);

#endif
