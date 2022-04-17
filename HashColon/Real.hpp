#ifndef HASHCOLON_REAL
#define HASHCOLON_REAL

// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace HashColon
{
	// Real definition
	// define real number as float / double
#if defined HASHCOLON_REAL_AS_DOUBLEPRECISION
	using Real = double;
#elif defined HASHCOLON_REAL_AS_SINGLEPRECISION
	using Real = float;
#else
	using Real = float;
#endif

	/* PI:
	 * Since M_PI is not C/C++ standard
	 * and std::numbers::pi is not supported until c++17,
	 * I need a pi constant
	 */
	struct Constant
	{
		inline static const Real PI = acos(-1.0);
		inline static const Real e = exp(1.0);
	};

	// void pointer
	using VoidPtr = void *;

	/// <summary>
	/// value with intervals
	/// </summary>
	template <typename valueTypeT>
	struct ValueInterval
	{
		ValueInterval() : minVal(), repVal(), maxVal(){};

		union
		{
			struct
			{
				valueTypeT minVal;
				valueTypeT repVal;
				valueTypeT maxVal;
			};
			std::array<valueTypeT, 3> val;
		};

		inline valueTypeT &operator[](std::size_t i) { return val[i]; };
	};

	// BoundedReal definition
	template <const Real &minVal, const Real &maxVal, bool isCircular = false>
	class BoundedReal;

	/// <summary>
	// Real values which has min/max boundary
	/// </summary>
	template <const Real &minVal, const Real &maxVal>
	class BoundedReal<minVal, maxVal, false>
	{
	protected:
		Real _val = 0.0;

	public:
		BoundedReal(){};
		BoundedReal(const Real &val);
		inline operator Real() const { return _val; };
		BoundedReal &operator=(Real val);
	};

	/// <summary>
	// Real values which circulates like degrees.
	/// </summary>
	template <const Real &minVal, const Real &maxVal>
	class BoundedReal<minVal, maxVal, true>
	{
	protected:
		Real _val = 0.0;

	public:
		BoundedReal(){};
		BoundedReal(const Real &val);
		inline operator Real() const { return _val; };
		BoundedReal &operator=(Real val);
		inline const Real toReal() { return _val; };
	};

	namespace _details
	{
		struct hiddenAnchor
		{
			inline static Real _minDegree = -180.0;
			inline static Real _maxDegree = 180.0;
			inline static Real _negativePI = -Constant::PI;
			inline static Real _positivePI = Constant::PI;

			inline static Real _realZero = 0.0;
			inline static Real _realTen = 10.0;
			inline static Real _realMax = std::numeric_limits<Real>::max();
			;
		};
	}

	class Radian;

	/// <summary>
	/// Angle in degrees. Circular value [0, 360)
	/// </summary>
	/// <seealso cref="BoundedReal{_minDegree, _maxDegree, true}" />
	class Degree : public BoundedReal<
					   _details::hiddenAnchor::_minDegree,
					   _details::hiddenAnchor::_maxDegree, true>
	{
	public:
		Degree() : BoundedReal(){};
		Degree(const Real &val) : BoundedReal(val){};

		friend inline bool operator==(const Degree &lhs, const Degree &rhs);
		friend inline bool operator!=(const Degree &lhs, const Degree &rhs);
		operator HashColon::Radian() const;
	};

	// radian definition
	class Radian : public BoundedReal<
					   _details::hiddenAnchor::_negativePI,
					   _details::hiddenAnchor::_positivePI, true>
	{
	public:
		Radian() : BoundedReal(){};
		Radian(const Real &val) : BoundedReal(val){};

		friend inline bool operator==(const Radian &lhs, const Radian &rhs);
		friend inline bool operator!=(const Radian &lhs, const Radian &rhs);
		operator HashColon::Degree() const;
	};

	// unsinged real definition
	using UnsignedReal = BoundedReal<
		_details::hiddenAnchor::_realZero,
		_details::hiddenAnchor::_realMax>;
}

// literals for Real
namespace HashColon
{
	// length unit: meter
	long double operator"" _m(long double m);	// meter is basic unit for length
	long double operator"" _mm(long double mm); // millimeter
	long double operator"" _cm(long double cm); // centimeter
	long double operator"" _km(long double km); // kilometer
	long double operator"" _in(long double in); // inch
	long double operator"" _ft(long double ft); // feet
	long double operator"" _mi(long double mi); // mile
	long double operator"" _yd(long double yd); // yard
	long double operator"" _NM(long double NM); // Nautical Mile

	// angle unit
	Radian operator"" _rad(long double rad); // radian
	Degree operator"" _deg(long double deg); // degree

	// weight unit: kilogram
	long double operator"" _kg(long double kg); // kilogram is basic unit for weight
	long double operator"" _mg(long double mg); // milligram
	long double operator"" _g(long double g);	// gram
	long double operator"" _t(long double t);	// ton
	long double operator"" _lb(long double lb); // lb(pound)

	// time unit: second
	long double operator"" _sec(long double sec); // second is basic unit for time
	long double operator"" _min(long double min); // minute
	long double operator"" _hr(long double hr);	  // hour
	long double operator"" _day(long double day); // day

	// speed unit: m/s (meter per second)
	long double operator"" _kn(long double kn);		// knots = NM per hour
	long double operator"" _mph(long double mph);	// mph = mile per hour
	long double operator"" _kmph(long double kmph); // kmph = kilometer per hour

	// pressure unit: pascal(Pa) = N/m^2
	long double operator"" _Pa(long double Pa);		// pascal
	long double operator"" _kPa(long double kPa);	// kilo-pascal
	long double operator"" _bar(long double bar);	// bar = 100 kPa
	long double operator"" _atm(long double atm);	// atmosphere
	long double operator"" _psi(long double psi);	// psi = pound per square inch
	long double operator"" _Torr(long double Torr); // Torr

	// Temperature unit: Celsius *** Despite SI unit of temperature is Kelvin, we are using Celsius.
	// Cuz We'll never use Kelvin for sure.
	long double operator"" _degC(long double degC); // Celcius degree
	long double operator"" _degF(long double degF); // Fahrenheight degree
	long double operator"" _Kelvin(long double K);	// Kelvin, (_K is c++ language reserved)
}

#endif

#ifdef EIGEN_WORLD_VERSION
#ifndef EIGEN_REAL_SUPPORT
#define EIGEN_REAL_SUPPORT
namespace Eigen
{
	template <int Size1, int Size2 = Size1>
	using MatrixR = Matrix<HashColon::Real, Size1, Size2>;
	using Matrix2R = Matrix<HashColon::Real, 2, 2>;
	using Matrix2XR = Matrix<HashColon::Real, 2, Dynamic>;
	using Matrix3R = Matrix<HashColon::Real, 3, 3>;
	using Matrix3XR = Matrix<HashColon::Real, 3, Dynamic>;
	using Matrix4R = Matrix<HashColon::Real, 4, 4>;
	using Matrix4XR = Matrix<HashColon::Real, 4, Dynamic>;
	using MatrixXR = Matrix<HashColon::Real, Dynamic, Dynamic>;
	using MatrixX2R = Matrix<HashColon::Real, Dynamic, 2>;
	using MatrixX3R = Matrix<HashColon::Real, Dynamic, 3>;
	using MatrixX4 = Matrix<HashColon::Real, Dynamic, 4>;

	template <int Size>
	using RowVectorR = Matrix<HashColon::Real, 1, Size>;
	using RowVector2R = Matrix<HashColon::Real, 1, 2>;
	using RowVector3R = Matrix<HashColon::Real, 1, 3>;
	using RowVector4R = Matrix<HashColon::Real, 1, 4>;
	using RowVectorXR = Matrix<HashColon::Real, 1, Dynamic>;

	template <int Size>
	using VectorR = Matrix<HashColon::Real, Size, 1>;
	using Vector2R = Matrix<HashColon::Real, 2, 1>;
	using Vector3R = Matrix<HashColon::Real, 3, 1>;
	using Vector4R = Matrix<HashColon::Real, 4, 1>;
	using VectorXR = Matrix<HashColon::Real, Dynamic, 1>;
}
#endif
#endif

#include <HashColon/impl/Real_Impl.hpp>