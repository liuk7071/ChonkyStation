set(SOURCE_FILES
"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/cpu/cpu.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/backends/interpreter/interpreter.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/memory/memory.cpp"
PARENT_SCOPE)

set(HEADER_FILES
"${CMAKE_CURRENT_SOURCE_DIR}/playstation.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/common/helpers.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/cpu/cpu_core.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/cpu.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/backends/interpreter/interpreter.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/memory/memory.hpp"
PARENT_SCOPE)