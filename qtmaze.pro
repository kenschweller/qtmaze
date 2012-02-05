TEMPLATE = app
TARGET = QtMaze
CONFIG += release
QT += opengl
HEADERS = \
	src/engine/defines.h \
	src/engine/Path.h \
	src/engine/Orientation.h \
	src/engine/Pictures.h \
	src/engine/WallTextures.h \
	src/engine/Walls.h \
	src/engine/Tiles.h \
	src/engine/Maze.h \
	src/engine/Camera.h \
	src/engine/PositionLogger.h \
	src/MazeWidget.h \
	src/NewDialog.h \
	src/PrefDialog.h \
	src/FilePane.h \
	src/DataModel.h \
	src/DataPane.h \
	src/ImagePane.h \
	src/MazePane.h \
	src/TrialTreeModel.h \
	src/TrialTreeView.h \
	src/TrialPane.h \
	src/QtMaze.h
SOURCES = \
	src/engine/Path.cpp \
	src/engine/Orientation.cpp \
	src/engine/Pictures.cpp \
	src/engine/WallTextures.cpp \
	src/engine/Walls.cpp \
	src/engine/Tiles.cpp \
	src/engine/Maze.cpp \
	src/engine/Camera.cpp \
	src/engine/PositionLogger.cpp \
	src/MazeWidget.cpp \
	src/NewDialog.cpp \
	src/PrefDialog.cpp \
	src/FilePane.cpp \
	src/DataModel.cpp \
	src/DataPane.cpp \
	src/ImagePane.cpp \
	src/MazePane.cpp \
	src/TrialTreeModel.cpp \
	src/TrialTreeView.cpp \
	src/TrialPane.cpp \
	src/QtMaze.cpp \
	src/main.cpp

Release:DESTDIR = ./
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

Debug:DESTDIR = debug/
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui

win32 {
CONFIG += console
QMAKE_CXXFLAGS += $(shell sdl-config --cflags)
INCLUDEPATH += C:/msys/mingw/include/SDL
QMAKE_CXXFLAGS += -D_GNU_SOURCE=1 -Dmain=SDL_main
LIBS += -lmingw32 -lSDLmain -lSDL
QMAKE_LFLAGS += -mwindows
}
macx {
INCLUDEPATH += /Library/Frameworks/SDL.framework/Headers
LIBS += -framework SDL
}
unix:!macs {
QMAKE_CXXFLAGS += $(shell sdl-config --cflags)
QMAKE_LFLAGS += $(shell sdl-config --libs)
}
