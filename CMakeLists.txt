cmake_minimum_required(VERSION 3.0)
project("Rack")

include_directories(./include ./dep ./include/dep ./dep/pffft)

add_subdirectory(test)
#add_subdirectory(dep)
