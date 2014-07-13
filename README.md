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
	make

### authors
Alex Borg
Robert Sebastian

