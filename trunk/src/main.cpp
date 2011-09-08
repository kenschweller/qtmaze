#include <QApplication>
#include "QtMaze.h"

#include <SDL.h>
#undef main

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE);
	QApplication app(argc, argv);

	QtMaze win;
	win.show();
	// win.showMaximized();

	const int result = app.exec();
	SDL_Quit();
	return result;
}
