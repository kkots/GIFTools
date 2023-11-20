# this CMakeLists.txt is for Linux compilation
# on Windows compile using Visual Studio's Build command
cmake_minimum_required(VERSION "${MIN_VER_CMAKE}" FATAL_ERROR)
project(remove_half_the_frames)
set(CMAKE_CXX_STANDARD 14)
add_executable(remove_half_the_frames remove_half_the_frames.cpp CrossPlatformDefs.h)
target_compile_definitions(remove_half_the_frames PRIVATE "-DFOR_LINUX=\"1\"")

# compile instructions
# cd into the directory with the CMakeLists.txt
#
# cmake .
# make
#
# The executable named "remove_half_the_frames" appears in the current directory.
# To launch, use:
#
# ./remove_half_the_frames /home/yourUser/image%.png 0-30