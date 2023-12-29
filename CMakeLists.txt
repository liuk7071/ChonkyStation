cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(ChonkyStation)
add_subdirectory(external)
add_subdirectory(test)
add_subdirectory(src)

include_directories(src/common)
include_directories(src/cpu)
include_directories(src/memory)
include_directories(src/dma)
include_directories(src/gpu)
include_directories(src/cdrom)
include_directories(src/scheduler)

include_directories(test)

include_directories(external/Dolphin)
include_directories(external/Panda3DS)

source_group("Header Files/Test" FILES ${TEST_HEADER_FILES})
source_group("Header Files/External/Dolphin" FILES ${DOLPHIN_HEADER_FILES})
source_group("Header Files/External/Panda3DS" FILES ${PANDA3DS_HEADER_FILES})

add_compile_definitions(SDL_MAIN_HANDLED)

add_executable(ChonkyStation ${SOURCE_FILES} ${HEADER_FILES} ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES} ${DOLPHIN_HEADER_FILES} ${PANDA3DS_HEADER_FILES})
target_include_directories(ChonkyStation PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(ChonkyStation PRIVATE fmt SDL2-static)