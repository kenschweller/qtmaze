#ifndef MAZEDEFINES_H
#define MAZEDEFINES_H

#include <QGLWidget>

static const int GRID_SIZE = 250;
// static const float GRID_SIZEF = static_cast<float>(GRID_SIZE);
static const float WALL_WIDTH = GRID_SIZE/8;
static const float HALF_WALL_WIDTH = WALL_WIDTH/2.0;
static const int LINE_WIDTH = 4;
static const int PICTURE_WIDTH = WALL_WIDTH;
static const float GOAL_RADIUS = 50.0;
static const float CAMERA_RADIUS = 50.0;

static const GLint     floorNormal[3] = { 0,  0, -1};
static const GLint   ceilingNormal[3] = { 0, -1,  0};
static const GLint  westWallNormal[3] = { 1,  0,  0};
static const GLint  eastWallNormal[3] = {-1,  0,  0};
static const GLint northWallNormal[3] = { 0,  0,  1};
static const GLint southWallNormal[3] = { 0,  0, -1};

#endif
