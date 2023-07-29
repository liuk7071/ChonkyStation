#pragma once
#include <cstdarg>
#include <fstream>
#include <string>

namespace Log {
// Our logger class
template <bool enabled>
class Logger {
public:
    Logger(std::string prepend = "") : prepend(prepend) {}

    std::string prepend;
    
    void log(const char* fmt, ...) {
        if constexpr (!enabled) return;

        std::fputs(prepend.c_str(), stdout);
        std::va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);
    }
};

// Our loggers here. Enable/disable by toggling the template param
static auto cpuTraceLogger  = Logger<false>("[CPU TRACE] ");
static auto dmaLogger       = Logger<true> ("[   DMA   ] ");
static auto gpuLogger       = Logger<true> ("[   GPU   ] ");


#define MAKE_LOG_FUNCTION(functionName, logger)          \
    template <typename... Args>                          \
    void functionName(const char* fmt, Args... args) {   \
        Log::logger.log(fmt, args...);                   \
    }
}