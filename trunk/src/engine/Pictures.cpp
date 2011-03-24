#include "Pictures.h"
#include "defines.h"

#include "GLee.h"

#include <QGLWidget>

static const QString picturesPath = "data/landmarks/";

bool Pictures::Picture::operator==(const Picture &other) const
{
	return (orientation == other.orientation && filename == other.filename);
}

bool Pictures::Picture::operator!=(const Picture &other) const
{
	return (orientation != other.orientation || filename != other.filename);
}

////////////////////

Pictures::Pictures() : context(NULL)
{
}

Pictures::~Pictures()
{
	_DeleteTextures();
}

Pictures & Pictures::operator=(const Pictures &other)
{
	clear();
	pictures = other.pictures;
	textures = other.textures;
	setContext(other.context);
	return *this;
}

bool Pictures::operator==(const Pictures &other) const
{
	return pictures == other.pictures;
}

bool Pictures::operator!=(const Pictures &other) const
{
	return pictures != other.pictures;
}

QTextStream & operator<<(QTextStream &out, const Pictures &pictures)
{
	for (Pictures::PictureMap::const_iterator it = pictures.pictures.begin(); it != pictures.pictures.end(); ++it)
	{
		out << "picture row " << it.value().orientation.tile.y() << " col " << it.value().orientation.tile.x() << " side " << it.value().orientation.directionToString() << " filename \"" << it.value().filename << "\"\n";
	}

	return out;
}

Pictures::Picture Pictures::at(const Orientation orientation) const
{
	PictureMap::const_iterator it = pictures.find(orientation);
	if (it != pictures.end())
		return it.value();
	return Picture();
}

void Pictures::add(const Orientation orientation, const QString &filename)
{
	remove(orientation);

	pictures.insert(orientation, Picture(orientation, filename));
	const TextureMap::iterator tex_it = textures.find(filename);
	if (tex_it == textures.end())
	{
		unsigned int textureID = 0;
		if (context)
		{
			const QString filePath = (picturesPath + filename);
			// printf("\tLoading '%s' ", filePath.toAscii().data());
			textureID = context->bindTexture(QPixmap(filePath));
			glBindTexture(GL_TEXTURE_2D, textureID);
			if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
			{
				float maximumAnisotropy = 1.1;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(4.0f, maximumAnisotropy));
			}
			glBindTexture(GL_TEXTURE_2D, 0);
			// printf("Loaded as %i\n", textureID);
		}
		textures.insert(filename, TextureEntry(textureID));
	}
	else
		tex_it.value().instanceCount++;
}

void Pictures::remove(const Orientation orientation)
{
	const PictureMap::iterator pic_it = pictures.find(orientation);
	if (pic_it == pictures.end())
		return;
	const TextureMap::iterator tex_it = textures.find(pic_it.value().filename);
	if (tex_it != textures.end())
	{
		tex_it.value().instanceCount--;
		if (tex_it.value().instanceCount <= 0)
		{
			// if (context)
				// context->deleteTexture(tex_it.value().textureObject);
			textures.erase(tex_it);
		}
	}
	pictures.erase(pic_it);
}

void Pictures::clear()
{
	pictures.clear();
	_DeleteTextures();
	textures.clear();
}

void Pictures::_DeleteTextures()
{
	if (context)
	{
		// for (TextureMap::iterator it = textures.begin(); it != textures.end(); ++it)
			// context->deleteTexture(it.value().textureObject);
	}
}

void Pictures::draw() const
{
	for (PictureMap::const_iterator it = pictures.begin(); it != pictures.end(); ++it)
		_DrawPicture(it.value());
}

void Pictures::setContext(QGLWidget *newContext)
{
	// printf("Switching context from %x to %x", (int)context, (int)newContext);
	_DeleteTextures();
	context = newContext;
	if (context)
	{
		for (TextureMap::iterator it = textures.begin(); it != textures.end(); ++it)
		{
			// printf("Loading '%s'\n", (picturesPath + it.key()).toAscii().data());
			it.value().textureObject = context->bindTexture(QPixmap(picturesPath + it.key()));
			glBindTexture(GL_TEXTURE_2D, it.value().textureObject);
			if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
			{
				float maximumAnisotropy = 1.1;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(4.0f, maximumAnisotropy));
			}
			glBindTexture(GL_TEXTURE_2D, 0);
			// printf("\tLoaded: %i\n", it.value().textureObject);
		}
	}
}
static const float PICTURE_MARGIN = GRID_SIZE/4.0;
void Pictures::_DrawPicture(const Picture &picture) const
{
	// TODO more intelligent behavior here
	const TextureMap::const_iterator tex_it = textures.find(picture.filename);
	if (tex_it == textures.end())
		return;

	// printf("Drawing '%s' -- %i\n", picture.filename.toAscii().data(), tex_it.value().textureObject);

	picture.orientation.draw();

	glBindTexture(GL_TEXTURE_2D, tex_it.value().textureObject);
	glBegin(GL_QUADS);
	switch (picture.orientation.direction)
	{
		case Orientation::North:
		glNormal3iv(northWallNormal);
		glTexCoord2i(0, 1); // upper north-west corner
		glVertex3i(picture.orientation.tile.x()*GRID_SIZE+HALF_WALL_WIDTH + PICTURE_MARGIN, picture.orientation.tile.y()*GRID_SIZE+HALF_WALL_WIDTH+1.0, -GRID_SIZE + PICTURE_MARGIN);
		glTexCoord2i(0, 0); // lower north-west corner
		glVertex3i(picture.orientation.tile.x()*GRID_SIZE+HALF_WALL_WIDTH + PICTURE_MARGIN, picture.orientation.tile.y()*GRID_SIZE+HALF_WALL_WIDTH+1.0, -PICTURE_MARGIN);
		glTexCoord2i(1, 0); // lower north-east corner
		glVertex3i((picture.orientation.tile.x()+1)*GRID_SIZE-HALF_WALL_WIDTH - PICTURE_MARGIN, picture.orientation.tile.y()*GRID_SIZE+HALF_WALL_WIDTH+1.0, -PICTURE_MARGIN);
		glTexCoord2i(1, 1); // upper north-east corner
		glVertex3i((picture.orientation.tile.x()+1)*GRID_SIZE-HALF_WALL_WIDTH - PICTURE_MARGIN, picture.orientation.tile.y()*GRID_SIZE+HALF_WALL_WIDTH+1.0, -GRID_SIZE + PICTURE_MARGIN);
		break;

		case Orientation::East:
		glNormal3iv(eastWallNormal);
		glTexCoord2i(0, 1); // upper north-east corner
		glVertex3i((picture.orientation.tile.x()+1)*GRID_SIZE-HALF_WALL_WIDTH-1.0, picture.orientation.tile.y()*GRID_SIZE+HALF_WALL_WIDTH + PICTURE_MARGIN, -GRID_SIZE + PICTURE_MARGIN);
		glTexCoord2i(0, 0); // lower north-east corner
		glVertex3i((picture.orientation.tile.x()+1)*GRID_SIZE-HALF_WALL_WIDTH-1.0, picture.orientation.tile.y()*GRID_SIZE+HALF_WALL_WIDTH + PICTURE_MARGIN, -PICTURE_MARGIN);
		glTexCoord2i(1, 0); // lower south-east corner
		glVertex3i((picture.orientation.tile.x()+1)*GRID_SIZE-HALF_WALL_WIDTH-1.0, (picture.orientation.tile.y()+1)*GRID_SIZE-HALF_WALL_WIDTH - PICTURE_MARGIN, -PICTURE_MARGIN);
		glTexCoord2i(1, 1); // upper south-east corner
		glVertex3i((picture.orientation.tile.x()+1)*GRID_SIZE-HALF_WALL_WIDTH-1.0, (picture.orientation.tile.y()+1)*GRID_SIZE-HALF_WALL_WIDTH - PICTURE_MARGIN, -GRID_SIZE + PICTURE_MARGIN);
		break;

		case Orientation::South:
		glNormal3iv(southWallNormal);
		glTexCoord2i(1, 1); // upper south-west corner
		glVertex3i(picture.orientation.tile.x()*GRID_SIZE+HALF_WALL_WIDTH + PICTURE_MARGIN, (picture.orientation.tile.y()+1)*GRID_SIZE-HALF_WALL_WIDTH-1.0, -GRID_SIZE + PICTURE_MARGIN);
		glTexCoord2i(0, 1); // upper south-east corner
		glVertex3i((picture.orientation.tile.x()+1)*GRID_SIZE-HALF_WALL_WIDTH - PICTURE_MARGIN, (picture.orientation.tile.y()+1)*GRID_SIZE-HALF_WALL_WIDTH-1.0, -GRID_SIZE + PICTURE_MARGIN);
		glTexCoord2i(0, 0); // lower south-east corner
		glVertex3i((picture.orientation.tile.x()+1)*GRID_SIZE-HALF_WALL_WIDTH - PICTURE_MARGIN, (picture.orientation.tile.y()+1)*GRID_SIZE-HALF_WALL_WIDTH-1.0, -PICTURE_MARGIN);
		glTexCoord2i(1, 0); // lower south-west corner
		glVertex3i(picture.orientation.tile.x()*GRID_SIZE+HALF_WALL_WIDTH + PICTURE_MARGIN, (picture.orientation.tile.y()+1)*GRID_SIZE-HALF_WALL_WIDTH-1.0, -PICTURE_MARGIN);
		break;

		case Orientation::West:
		glNormal3iv(westWallNormal);
		glTexCoord2i(1, 1); // upper north-west corner
		glVertex3i(picture.orientation.tile.x()*GRID_SIZE+HALF_WALL_WIDTH+1.0, picture.orientation.tile.y()*GRID_SIZE+HALF_WALL_WIDTH + PICTURE_MARGIN, -GRID_SIZE + PICTURE_MARGIN);
		glTexCoord2i(0, 1); // upper south-west corner
		glVertex3i(picture.orientation.tile.x()*GRID_SIZE+HALF_WALL_WIDTH+1.0, (picture.orientation.tile.y()+1)*GRID_SIZE-HALF_WALL_WIDTH - PICTURE_MARGIN, -GRID_SIZE + PICTURE_MARGIN);
		glTexCoord2i(0, 0); // lower south-west corner
		glVertex3i(picture.orientation.tile.x()*GRID_SIZE+HALF_WALL_WIDTH+1.0, (picture.orientation.tile.y()+1)*GRID_SIZE-HALF_WALL_WIDTH - PICTURE_MARGIN, -PICTURE_MARGIN);
		glTexCoord2i(1, 0); // lower north-west corner
		glVertex3i(picture.orientation.tile.x()*GRID_SIZE+HALF_WALL_WIDTH+1.0, picture.orientation.tile.y()*GRID_SIZE+HALF_WALL_WIDTH + PICTURE_MARGIN, -PICTURE_MARGIN);
		break;
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}
