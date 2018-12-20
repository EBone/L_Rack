cmake_minimum_required(VERSION 3.0)
project("Rack")

Set(LIBZIPDIR "E:/Libs/libzip")
Set(LIBOpenSllDIR "E:/Libs/openssl")
Set(LIBNanoVgDir "E:/Libs/nanovg")
Set(LIBZlibDir "E:/Libs/zlib")
Set(LIBCurlDir "E:/Libs/libcurl")
Set(LIBSpeexDspDir "E:/Codes/L_Rack/dep/speexdsp/lib")
include_directories(./include ./dep ./include/dep ./dep/pffft ./dep/glfw/include/GLFW ./dep/glew/include)
include_directories(./dep/nanovg/src ./dep/rtaudio/include ./dep/nanosvg/src ./dep/jansson/src)
include_directories(./dep/oui-blendish)
include_directories(./dep/jansson/src )
include_directories(./dep/osdialog )
include_directories(${PROJECT_BINARY_DIR}/dep/jansson/include ${PROJECT_BINARY_DIR}/dep/jansson/private_include)
include_directories(./dep/rtaudio)
include_directories(./dep/rtmidi)
include_directories(./dep/speexdsp/include)
include_directories(./dep/curl/include)
include_directories(${LIBOpenSllDIR}/inc32)
include_directories(${LIBZIPDIR}/include)

link_directories(${LIBZIPDIR}/lib ${LIBOpenSllDIR} ${LIBZlibDir})
link_directories(${LIBNanoVgDir})
link_directories(${LIBCurlDir})
link_directories(${LIBSpeexDspDir})

file(GLOB rack_include include/*.hpp)
file(GLOB rack_dsp_include include/dsp/*.hpp)
file(GLOB rack_util_include include/util/*.hpp)

file(GLOB rack_src src/*.cpp src/*.c src/*.h)
file(GLOB rack_dsp_src src/dsp/*.cpp)
file(GLOB rack_util_src src/util/*.cpp)
file(GLOB rack_app_src src/app/*.cpp)
file(GLOB rack_Core_src src/Core/*.cpp src/Core/*.hpp)
file(GLOB rack_widgets_src src/widgets/*.cpp)
file(GLOB glfw_deps ./dep/glfw/deps/*.h ./dep/glfw/deps/*.cpp)
file(GLOB rack_ui_src src/ui/*.cpp)

source_group(rack_include FILES ${rack_include})
source_group(rack_dsp_include FILES ${rack_dsp_include})
source_group(rack_util_include FILES ${rack_util_include})
source_group(rack_include FILES ${rack_include})
source_group(rack_src FILES ${rack_src})
source_group(rack_dsp_src FILES ${rack_dsp_src})
source_group(rack_util_src FILES ${rack_util_src})
source_group(rack_app_src FILES ${rack_app_src})
source_group(rack_Core_src FILES ${rack_Core_src})
source_group(rack_widgets_src FILES ${rack_widgets_src})
source_group(rack_ui_src FILES ${rack_ui_src})

add_subdirectory(dep)


add_executable(rack_exe src/main/main.cpp ${rack_include} ${rack_dsp_include} ${rack_util_include} 
${rack_util_include} ${rack_include} ${rack_src} ${rack_dsp_src} ${rack_util_src} ${rack_app_src} 
${rack_Core_src} ${rack_widgets_src} ${glfw_deps} ${rack_ui_src} dep/rtaudio/RtAudio.cpp dep/rtmidi/RtMidi.cpp)
add_library(rack_lib STATIC  ${rack_include} ${rack_dsp_include} ${rack_util_include} 
${rack_util_include} ${rack_include} ${rack_src} ${rack_dsp_src} ${rack_util_src} ${rack_app_src} 
${rack_Core_src} ${rack_widgets_src} ${glfw_deps} ${rack_ui_src} dep/rtaudio/RtAudio.cpp dep/rtmidi/RtMidi.cpp)

target_link_libraries(rack_exe pffft  jansson glfw glew OpenGl32 osdialog zip nanovg libeay32 ssleay32 WS2_32 libcurl Wldap32 libspeexdsp)
target_link_libraries(rack_lib pffft  jansson glfw glew OpenGl32 osdialog zip nanovg libeay32 ssleay32 WS2_32 libcurl Wldap32 libspeexdsp)
add_subdirectory(test)
add_subdirectory(plugins)


