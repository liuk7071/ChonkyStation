set(SOURCE_FILES
"main.cpp"

"cpu/cpu.cpp"
"cpu/backends/interpreter/interpreter.cpp"

"memory/memory.cpp"
)

set(HEADER_FILES
"playstation.hpp"

"common/helpers.hpp"

"cpu/cpu_core.hpp"
"cpu/cpu.hpp"
"cpu/backends/interpreter/interpreter.hpp"

"memory/memory.hpp"
)

include_directories(common)
include_directories(cpu)
include_directories(memory)

add_executable(ChonkyStation ${SOURCE_FILES} ${HEADER_FILES})