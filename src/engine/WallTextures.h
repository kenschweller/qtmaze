#ifndef WALLTEXTURES_H
#define WALLTEXTURES_H

#include <QMap>
#include <QPair>
#include <QString>

class QGLWidget;

class WallTextures
{
public:
	typedef QMap<quint8, unsigned int> InternalIdToIdMap;
	typedef QMap<quint8, QString> IdToNameMap;
public:
	WallTextures();
	~WallTextures();

	const InternalIdToIdMap & texturesAvailable() const {return _texturesUsed;}
	const IdToNameMap & textureNames() const {return names;}

	void setContext(QGLWidget *newContext);

	void clear();
	void mapTexture(quint8 id, const QString &filename);
	quint8 addTexture(const QString &filename);
	quint8 getInternalTextureId(const QString &filename) const;
	unsigned int getTextureId(const QString &filename) const;
protected:
	quint8 _FindFreeId();

	typedef QMap<QString, quint8> NameToIdMap;
	typedef QMap<QString, unsigned int> NameToOGLIdMap;

	QGLWidget *_context;
	NameToIdMap ids;
	IdToNameMap names;
	NameToOGLIdMap textures;
	InternalIdToIdMap _texturesUsed;
	int nextId;
};

#endif
