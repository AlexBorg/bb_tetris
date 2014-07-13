# Make sure the compiler can find include files from our 3rd party library. 
include_directories (${XENOMAI_INC_DIR}) 

# Make sure the linker can find the 3rd party libraries. 
link_directories (${BBT_SOURCE_DIR}/3rdparty/lib/ ) 

# Add executable called "bbcd" that is built from the source files 
# "ioTest.cpp". The extensions are automatically found. 
add_executable (bbt main.cpp InputHandler.cpp) 

# Link the executable to the 3rdparty library. 
target_link_libraries (bbt pthread rt ) 

