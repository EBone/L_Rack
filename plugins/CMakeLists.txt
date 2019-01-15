project(Plug_Jw)

file(GLOB jw_source src/*.cpp src/*.hpp)
#file(GLOB widgets ../src/widgets/*.cpp ../src/app/*.cpp)
#file(GLOB rack_src ../src/*.cpp ../src/*.c ../src/*.h)
add_executable(jw ${jw_source} ../src/main/main.cpp)
add_library(plugin SHARED  ${jw_source})
target_link_libraries(jw rack_lib pffft  jansson glfw glew OpenGl32 nanovg)
target_link_libraries(plugin rack_lib) #pffft  jansson glfw glew OpenGl32 nanovg)