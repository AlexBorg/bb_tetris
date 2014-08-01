# beaglebone tetris

A project to create a tetris clone running on a beaglebone black using xenomai to demonstrate realtime operation. 

### Resources


### Compiling
It is highly reccomended that you create a build subdirectory at the top level of the source and run cmake from there.
	mkdir build
	cd build

On the beagleboard: requires cmake, opencv, and xenomai
	cmake <target source dir>
	make

options that can be added:
	cmake -DGCC_COMPILER_VERSION=4.8 -DXENOMAI_BASE_DIR=<copy of /usr/xenomai> <target source dir>
        cmake -DNOXENOMAI
	make

### authors
Alex Borg
Robert Sebastian

# Project Overview

## Operation

The program consists of three main segments: Input, Game Logic, and Display.
Whenever possible we used posix hooks to real time functions.
To facilitate testing, some xenomai only functions have alternates defined if the NOXENOMAI compile option is selected.

### Input

The InputHandler (InputHandler.cpp & InputHandler.hpp) class deals with all inputs to the system.
It captures inputs from all keyboards or playstation controllers connected to the system.
For each device found in /dev/input/event* it creates a new thread using the thread_func functions.
Each thread opens the device and and starts blocking reads for events.
Each event is delivered to a posix queue to be passed to the Game logic thread.

### Game Logic

The GameController (GameController.cpp & GameController.hpp) class controls all game logic.
The processTick function is called periodically (60 Hz).
ProcessTick handles all events from the input thread one at a time, and then updates the game board state for each.

### diaplay

The DipslayHandler class (DisaplayHandler.cpp & DisplayHandler.hpp) handles drawing to the screen.
It uses the SDL library to setup windows and create an OpenGL context.
The loop pulls the game state from the GameController and draws the data to the screen through openGL calls.
Because the BeagleBone's graphics capabilities are so slow, the display handler only draws a block if the block has changed.
