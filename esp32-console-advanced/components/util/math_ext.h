/**
 * Math Functions
 * @author Seth Teichman
 */
#pragma once
#define MATH_EXT_MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MATH_EXT_MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MATH_EXT_CLAMP(value, min, max) (MATH_EXT_MIN((max), MATH_EXT_MAX((min), (value))))