# Make sure the compiler can find include files from our default source tree library. 
include_directories (${BBT_SOURCE_DIR}/src ${XENOMAI_INC_DIR}) 

# Make sure the linker can find the 3rd party libraries. 
link_directories (${BBT_BINARY_DIR} ${BBT_SOURCE_DIR}/3rdparty/lib ${XENOMAI_LIB_DIR}) 

# Add executable called "test_name" that is built from the source files 
# "test_source.cpp". The extensions are automatically found. 
add_executable (input_test input_test.cpp ${BBT_SOURCE_DIR}/src/InputHandler.cpp) 

# Link the executable to the library. 
target_link_libraries (input_test native xenomai pthread rt) 

