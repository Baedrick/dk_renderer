/*
 * Copyright (C) 2026 Koh Swee Teck Dedrick.
 * Licensed under the Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
 */

#include "base/base.hpp"

#include "base/base.cpp"

auto entry_point(int argc, char **argv) noexcept -> int {
	(void)argc;
	(void)argv;

	return 0;
}

// TODO(Dedrick): Move main to platform initialization.
int main(int argc, char **argv) {
	return entry_point(argc, argv);
}
