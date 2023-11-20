# this CMakeLists.txt is for Linux compilation
# on Windows compile using Visual Studio's Build command
cmake_minimum_required(VERSION "${MIN_VER_CMAKE}" FATAL_ERROR)
project(change_gif_durations)
set(CMAKE_CXX_STANDARD 14)
add_executable(change_gif_durations change_gif_durations.cpp GIF_parse.h GIF_parse.cpp CrossPlatformDefs.h)
target_compile_definitions(change_gif_durations PRIVATE "-DFOR_LINUX=\"1\"")

# compile instructions
# cd into the directory with the CMakeLists.txt
#
# cmake .
# make
#
# The executable named "change_gif_durations" appears in the current directory.
# To launch, use:
#
# ./change_gif_durations --help