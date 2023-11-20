# this CMakeLists.txt is for Linux compilation
# on Windows compile using Visual Studio's Build command
cmake_minimum_required(VERSION "${MIN_VER_CMAKE}" FATAL_ERROR)
project(renumber_frames)
set(CMAKE_CXX_STANDARD 14)
add_executable(renumber_frames renumber_frames.cpp CrossPlatformDefs.h)
target_compile_definitions(renumber_frames PRIVATE "-DFOR_LINUX=\"1\"")

# compile instructions
# cd into the directory with the CMakeLists.txt
#
# cmake .
# make
#
# The executable named "renumber_frames" appears in the current directory.
# To launch, use:
#
# ./renumber_frames /home/yourUser/image%.png 0-30 31