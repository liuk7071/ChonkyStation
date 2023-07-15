#pragma once

#include <stdio.h>
#include <cstdint>
#include <cstdarg>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


// Types
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8  = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using uptr = std::uintptr_t;

constexpr size_t operator""KB(unsigned long long int x) { return 1024ULL * x; }
constexpr size_t operator""MB(unsigned long long int x) { return 1024KB * x; }
constexpr size_t operator""GB(unsigned long long int x) { return 1024MB * x; }


namespace Helpers {
	[[noreturn]] static void panic(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		exit(0);
	}

	static auto readBinary(std::string directory) -> std::vector<u8> {
		std::ifstream file(directory, std::ios::binary);
		if (!file.is_open()) {
			std::cout << "Couldn't find ROM at " << directory << "\n";
			exit(1);
		}

		std::vector<uint8_t> binary;
		file.unsetf(std::ios::skipws);
		std::streampos fileSize;
		file.seekg(0, std::ios::end);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		binary.insert(binary.begin(),
			std::istream_iterator<uint8_t>(file),
			std::istream_iterator<uint8_t>());
		file.close();

		return binary;
	}

	static void dump(const char* filename, u8* data, u64 size) {
		std::ofstream file(filename, std::ios::binary);
		file.write((const char*)data, size);
	}

	template<typename T>
	static inline bool inRange(T num, T start, T end) {
		if ((start <= num) && (num <= end)) return true;
		else return false;
	}

	template<typename T>
	static inline bool inRangeSized(T num, T start, T size) {
		if ((start <= num) && (num < (start + size))) return true;
		else return false;
	}
}	// End namespace Helpers