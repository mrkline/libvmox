#pragma once

/// A pixel offset with x, y, and pointer offset components
struct PixelOffset {
	int x; ///< X offset
	int y; ///< Y offset
	int p; ///< Pointer offset

	PixelOffset(int x, int y, size_t p) : x(x), y(y), p(p) { }
};
