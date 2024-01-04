#pragma once

#include <helpers.hpp>
#include "../../src/playstation.hpp"


namespace Test {
	class Diff {
	public:
		Diff(const fs::path& biosPath, const fs::path& cdPath);
		void doTest();
	private:
		PlayStation p1;
		PlayStation p2;
	};
}	// End namespace test