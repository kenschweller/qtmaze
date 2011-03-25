#! /bin/sh

# Build & run QtMaze on Mac OS X

qmake -spec macx-g++
make
open QtMaze.app

