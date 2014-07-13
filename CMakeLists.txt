# The name of our project is "BBT". This is short for "BeagleBone Tetris"
# CMakeLists files in this project can 
# refer to the root source directory of the project as ${BBT_SOURCE_DIR} and 
# to the root binary directory of the project as ${BBT_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.6) 
project (BBT) 

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(GCC_COMPILER_VERSION "4.6" CACHE STRING "GCC Compiler version")
set(XENOMAI_BASE_DIR "/usr/xenomai" CACHE STRING "Xenomai base dir")
set(XENOMAI_INC_DIR "${XENOMAI_BASE_DIR}/include")
set(XENOMAI_LIB_DIR "${XENOMAI_BASE_DIR}/lib")

set(FLOAT_ABI_SUFFIX "")

if (NOT SOFTFP)
  set(FLOAT_ABI_SUFFIX "hf")
endif()

if (ARM)
  set(CMAKE_C_COMPILER    arm-linux-gnueabi${FLOAT_ABI_SUFFIX}-gcc-${GCC_COMPILER_VERSION})
  set(CMAKE_CXX_COMPILER  arm-linux-gnueabi${FLOAT_ABI_SUFFIX}-g++-${GCC_COMPILER_VERSION})
  set(ARM_LINUX_SYSROOT /usr/arm-linux-gnueabi${FLOAT_ABI_SUFFIX} CACHE PATH "ARM cross compilation system root")
endif()

MESSAGE( STATUS "")
MESSAGE( STATUS "  C/C++:")
MESSAGE( STATUS "    Built as dynamic libs?:" BUILD_SHARED_LIBS THEN YES ELSE NO)
MESSAGE( STATUS "    C++ Compiler:"           ${CMAKE_CXX_COMPILER})
MESSAGE( STATUS "    C++ flags (Release):"    ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE})
MESSAGE( STATUS "    C++ flags (Debug):"      ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG})
MESSAGE( STATUS "    C Compiler:"             ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1})
MESSAGE( STATUS "    C flags (Release):"      ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE})
MESSAGE( STATUS "    C flags (Debug):"        ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG})
if(WIN32)
  MESSAGE( STATUS "    Linker flags (Release):" ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
  MESSAGE( STATUS "    Linker flags (Debug):"   ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
else()
  MESSAGE( STATUS "    Linker flags (Release):" ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
  MESSAGE( STATUS "    Linker flags (Debug):"   ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
endif()
MESSAGE( STATUS "    Precompiled headers:"     PCHSupport_FOUND AND ENABLE_PRECOMPILED_HEADERS THEN YES ELSE NO)

MESSAGE( STATUS "")
MESSAGE( STATUS "BBT_SOURCE_DIR:" ${BBT_SOURCE_DIR})
MESSAGE( STATUS "BBT_BINARY_DIR:" ${BBT_BINARY_DIR})
MESSAGE( STATUS "XENOMAI_BASE_DIR:" ${XENOMAI_BASE_DIR})
MESSAGE( STATUS "XENOMAI_INC_DIR:" ${XENOMAI_INC_DIR})
MESSAGE( STATUS "XENOMAI_LIB_DIR:" ${XENOMAI_LIB_DIR})
MESSAGE( STATUS "")

# Recurse into the "src" and "test" subdirectories. This does not actually 
# cause another cmake executable to run. The same process will walk through 
# the project's entire directory structure. 
add_subdirectory (src) 
add_subdirectory (test)
