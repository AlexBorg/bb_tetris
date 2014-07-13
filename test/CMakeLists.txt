# Make sure the compiler can find include files from our default source tree library. 
include_directories (${BBT_SOURCE_DIR}/3rdparty/include) 

# Make sure the linker can find the 3rd party libraries. 
link_directories (${BBT_SOURCE_DIR}/3rdparty/lib) 

# Add executable called "test_name" that is built from the source files 
# "test_source.cpp". The extensions are automatically found. 
#add_executable (test_name test_source.cpp) 

# Link the executable to the library. 
#target_link_libraries (test_name Library_name) 

