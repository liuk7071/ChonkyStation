cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(ChonkyStation)
add_subdirectory(external)
add_subdirectory(src)

include_directories(src/common)
include_directories(src/cpu)
include_directories(src/memory)
include_directories(src/dma)
include_directories(src/gpu)

include_directories(external/Dolphin)
include_directories(external/Panda3DS)

source_group("Header Files/External/Dolphin" FILES ${DOLPHIN_HEADER_FILES})
source_group("Header Files/External/Panda3DS" FILES ${PANDA3DS_HEADER_FILES})

add_compile_definitions(SDL_MAIN_HANDLED)

if (ANDROID)
    add_library(ChonkyStation SHARED ${SOURCE_FILES} ${HEADER_FILES} ${DOLPHIN_HEADER_FILES} ${PANDA3DS_HEADER_FILES} android/app/src/main/cpp/AndroidOut.cpp)
    find_package(game-activity REQUIRED CONFIG)
else()
    add_executable(ChonkyStation ${SOURCE_FILES} ${HEADER_FILES} ${DOLPHIN_HEADER_FILES} ${PANDA3DS_HEADER_FILES})
endif()


target_include_directories(ChonkyStation PRIVATE ${SDL2_INCLUDE_DIR})

if (NOT ANDROID)
    target_link_libraries(ChonkyStation PRIVATE fmt SDL2-static)
else()
    target_link_libraries(ChonkyStation PRIVATE fmt game-activity::game-activity EGL GLESv3 jnigraphics android log)
endif()