// Taken from MKb: https://github.com/slavik262/MKb under the zlib license

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

/**
\brief Contains constants and functions for common math operations.

Operations already contained in the C++ STL are not re-implemented here.
The STL should be used  directly for such operations. Floating point equality
operations are borrowed from
http://altdevblogaday.com/2012/02/22/comparing-floating-point-numbers-2012-edition/
*/
namespace Math {
	/// Float representation of Pi
	const float kPi	= 3.14159265359f;
	/// Float representation of 1 / Pi
	const float kPiRecip = 1.0f / kPi;
	/// Float representation of Pi / 2
	const float kHalfPi	= kPi / 2.0f;
	/// Double representation of Pi
	const double kPiDouble = 3.1415926535897932384626433832795028841971693993751;
	/// Double representation of 1 / Pi
	const double kPiDoubleRecip = 1.0 / kPiDouble;
	/// Double represntation of Pi / 2
	const double kHalfPiDouble = kPiDouble / 2.0;
	/// Conversion ratio to convert angle measurements in
	/// degrees to radian angle mesurements
	const float kDegToRad = kPi / 180.0f;
	/// Conversion ratio to convert angle measurements in
	/// radians to degree angle mesurements
	const float kRadToDeg = 180.0f / kPi;
	/// Conversion ratio to convert angle measurements in
	/// degrees to radian angle mesurements
	const double kDegToRadDouble = kPiDouble / 180.0;
	/// Conversion ratio to convert angle measurements in
	/// radians to degree angle mesurements
	const double kRadToDegDouble = 180.0 / kPiDouble;
	/// Default number of Ulps considered for floating-point equality
	const int kUlpsEquality = 2;

	/**
	\brief Returns the sign of a value
	\return The sign of val (-1, 0, or 1)

	From http://stackoverflow.com/a/4609795/713961
	*/
	template <class T>
	inline int sign(const T& value) {
		return (T(0) < value) - (value < T(0));
	}

	/**
	\brief Clamps a value between a low and high value
	\param value The value to clamp
	\param low The minimum return value
	\param high The maximum return value
	\return low <= return value <= high

	Clamps a value between a low and high value using std::min and std::max
	*/
	template <class T>
	inline const T clamp(const T& value, const T& low, const T& high)
	{
		return std::min(std::max(value, low), high);
	}

	/// Lerps between two values based on parameter t
	template <class T>
	inline const T lerp(const T& a, const T& b, const T& t)
	{
		return a + (b - a) * t;
	}

	/**
	\brief Tests two floats for equality within a given tolerance
	\param a The first value to test for equality
	\param b The second value to test for equality
	\param tolerance The tolerance allowed between a and b for them to
			still be considered equal
	\return a == b, within tolerance

	This allows for safer floating comparisons,
	since direct ones may work poorly due to rounding error.
	*/
	inline bool equals(float a, float b, int tolerance = kUlpsEquality)
	{
		union FloatUnion
		{
			FloatUnion(float flt) : f(flt) { }
			bool isPositive() const { return (i >> 31) != 0; }
			int32_t i;
			float f;
		};

		FloatUnion uA(a);
		FloatUnion uB(b);

		if (uA.isPositive() == uB.isPositive())
			 return abs(uA.i - uB.i) <= tolerance;
		else
			return a == b;
	}

	/**
	\brief Tests two doubles for equality within a given tolerance
	\param a The first value to test for equality
	\param b The second value to test for equality
	\param tolerance The tolerance allowed between a and b for them to
			still be considered equal
	\return a == b, within tolerance

	This allows for safer floating comparisons,
	since direct ones may work poorly due to rounding error.
	*/
	inline bool equals(double a, double b, int tolerance = kUlpsEquality)
	{
		union DoubleUnion
		{
			DoubleUnion(double dub) : d(dub) { }
			bool isPositive() const { return (i >> 63) != 0; }
			int64_t i;
			double d;
		};

		DoubleUnion uA(a);
		DoubleUnion uB(b);

		if (uA.isPositive() == uB.isPositive())
			 return abs(uA.i - uB.i) <= tolerance;
		else
			return a == b;
	}

	/**
	\brief Tests a float for equality to zero
	\param a The first value to test for equality to zero
	\param tolerance The tolerance allowed between 0 and a for a to
			still be considered 0
	\return a == 0.0f, within tolerance
	\see Equals(float, float, float)

	This allows for safer floating comparisons,
	since direct ones may work poorly due to rounding error.
	*/
	inline bool isZero(float a, int tolerance = kUlpsEquality)
	{
		return equals(a, 0.0f, tolerance);
	}

	/**
	\brief Tests a double for equality to zero
	\param a The first value to test for equality to zero
	\param tolerance The tolerance allowed between 0 and a for a to
			still be considered 0
	\return a == 0.0, within tolerance
	\see Equals(double, double, double)

	This allows for safer floating comparisons,
	since direct ones may work poorly due to rounding error.
	*/
	inline bool isZero(double a, int tolerance = kUlpsEquality)
	{
		return equals(a, 0.0, tolerance);
	}

} // end namespace Math
