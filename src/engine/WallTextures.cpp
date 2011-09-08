#include "WallTextures.h"

#include <QGLWidget>
#include "GLee.h"

WallTextures::WallTextures() : _context(NULL), nextId(0)
{
	// HACK HACK HACK!
	addTexture("wall.jpg");
	// addTexture("bricks_00.jpg");
	// addTexture("bricks_01.jpg");
}

WallTextures::~WallTextures()
{
	setContext(NULL);
}
// #include <QDebug>
void WallTextures::setContext(QGLWidget *newContext)
{
	if (_context)
	{
		// delete all loaded textures
		// HACK
		/*for (NameToOGLIdMap::const_iterator it = textures.begin(); it != textures.end(); ++it)
			_context->deleteTexture(*it);*/
		textures.clear();
		for (InternalIdToIdMap::iterator it = _texturesUsed.begin(); it != _texturesUsed.end(); ++it)
			*it = 0;
	}
	_context = newContext;
	if (_context)
	{
		const bool anisotropicAvailable = strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic");
		float maximumAnisotropy = 1.1;
		if (anisotropicAvailable)
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
		// load any textures we are actually using
		for (NameToIdMap::const_iterator it = ids.begin(); it != ids.end(); ++it)
		{
			const int internalId = ids.value(it.key());
			const int openglId = _context->bindTexture(QPixmap(QString("data/textures/") + it.key()));
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);
			textures.insert(it.key(), openglId);
			_texturesUsed[internalId] = openglId;
			// qDebug() << "setContext:" << (QString("data/textures/") + it.key()) << internalId << openglId;
		}
	}
}

void WallTextures::clear()
{
	ids.clear();
	names.clear();
	_texturesUsed.clear();
	nextId = 0;

	addTexture("wall.jpg");
	// addTexture("bricks_00.jpg");
	// addTexture("bricks_01.jpg");
}

void WallTextures::mapTexture(quint8 id, const QString &filename)
{
	// see if this texture is already loaded, but with a different internal id
	NameToIdMap::iterator it = ids.find(filename);
	if (it != ids.end())
	{
		// check to see if it's already mapped to the desired internal id
		if (*it == id)
			return;

		// otherwise delete that mapping and create a new mapping
		names.remove(*it);
		_texturesUsed.remove(*it);
		*it = id;
		names[id] = filename;
	}
	else
	{
		// associate the filename with the id
		ids.insert(filename, id);
		names.insert(id, filename);
	}

	// make sure we have the texture loaded by filename
	if (!textures.contains(filename) && _context)
	{
		const unsigned int id = _context->bindTexture(QPixmap(QString("data/textures/") + filename));
		textures.insert(filename, id);
		if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
		{
			glBindTexture(GL_TEXTURE_2D, id);
			float maximumAnisotropy = 1.1;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	_texturesUsed[id] = textures.value(filename, 0);
}

quint8 WallTextures::addTexture(const QString &filename)
{
	// see if the texture is already in use
	NameToIdMap::iterator it = ids.find(filename);
	if (it != ids.end())
		return *it;

	// otherwise generate an id for it and possibly load it
	const int newId = _FindFreeId();
	if (newId == 255)
		return 0; // HACK
	ids.insert(filename, newId);
	names.insert(newId, filename);

	if (!textures.contains(filename) && _context)
	{
		const unsigned int id = _context->bindTexture(QPixmap(QString("data/textures/") + filename));
		textures.insert(filename, id);
		if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
		{
			glBindTexture(GL_TEXTURE_2D, id);
			float maximumAnisotropy = 1.1;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	_texturesUsed[newId] = textures.value(filename, 0);

	return newId;
}

quint8 WallTextures::getInternalTextureId(const QString &filename) const
{
	return ids.value(filename, ids.empty() ? 0 : *ids.begin());
}

unsigned int WallTextures::getTextureId(const QString &filename) const
{
	return textures.value(filename, 0);
}

quint8 WallTextures::_FindFreeId()
{
	for (int i = 0; i < 255; i++, nextId = (nextId + 1) % 255)
	{
		if (!names.contains(nextId))
		{
			const int newId = nextId;
			nextId = (nextId + 1) % 255;
			return newId;
		}
	}
	return 255;
}
