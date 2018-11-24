cmake_minimum_required(VERSION 3.0)

file(GLOB wdl_h ../wdl/*.h)
file(GLOB wdl_src ../wdl/*.cpp ../wdl/*.c)
include_directories(src ../dep/glfw/include ../dep/nanosvg/src)
file(GLOB nano_c ../dep/nanosvg/sr/*.c)
file(GLOB nano_h ../dep/nanosvg/src/*.h)
source_group(wdl\\wdl_h FILES ${wdl_h})
source_group(wdl\\wdl_src FILES ${wdl_src})
source_group(nanosvg FILES ${nano_c} ${nano_h})

add_executable(test main.cpp ${wdl_h} ${wdl_src} ${nano_c} ${nano_h})


target_link_libraries(test pffft glfw OpenGl32)