#pragma once
#include <cstdarg>
#include <fstream>
#include <string>
#ifdef __ANDROID__
#include "../../android/app/src/main/cpp/AndroidOut.h"
#endif


namespace Log {
// Our logger class
template <bool enabled>
class Logger {
public:
    Logger(std::string prepend = "") : prepend(prepend) {}

    std::string prepend;
    
    void log(const char* fmt, ...) {
        if constexpr (!enabled) return;

#ifndef __ANDROID__
        std::fputs(prepend.c_str(), stdout);
        std::va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);
#else
        aout << prepend.c_str();
        va_list args;
		va_start(args, fmt);
		char* out = new char[256];
		vsprintf(out, fmt, args);
		aout << out << std::endl;
		va_end(args);
#endif
    }
};

// Our loggers here. Enable/disable by toggling the template param
static auto cpuTraceLogger  = Logger<false>("[CPU TRACE] ");
static auto dmaLogger       = Logger<false>("[   DMA   ] ");
static auto gpuLogger       = Logger<false>("[   GPU   ] ");


#define MAKE_LOG_FUNCTION(functionName, logger)          \
    template <typename... Args>                          \
    void functionName(const char* fmt, Args... args) {   \
        Log::logger.log(fmt, args...);                   \
    }
}