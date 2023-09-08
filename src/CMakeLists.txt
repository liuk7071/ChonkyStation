set(SOURCE_FILES
"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/cpu/cpu.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/backends/interpreter/interpreter.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/backends/old_interpreter/old_interpreter.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/interrupt.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/cpu/disassembler/disassembler.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/memory/memory.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/dma/dma.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/gpu/gpu.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/gpu/backends/software/gpu_software.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/cdrom/cdrom.cpp"

"${CMAKE_CURRENT_SOURCE_DIR}/scheduler/scheduler.cpp"
PARENT_SCOPE)

set(HEADER_FILES
"${CMAKE_CURRENT_SOURCE_DIR}/playstation.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/common/helpers.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/cpu/cpu_core.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/cpu.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/backends/interpreter/interpreter.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/backends/old_interpreter/old_interpreter.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/cpu/interrupt.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/cpu/disassembler/disassembler.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/memory/memory.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/dma/dma.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/gpu/gpu.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/gpu/backends/base.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/gpu/backends/vertex.hpp"
"${CMAKE_CURRENT_SOURCE_DIR}/gpu/backends/software/gpu_software.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/cdrom/cdrom.hpp"

"${CMAKE_CURRENT_SOURCE_DIR}/scheduler/scheduler.hpp"
PARENT_SCOPE)