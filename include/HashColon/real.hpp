#ifndef _HG_HASHCOLON_REAL
#define _HG_HASHCOLON_REAL

#include <HashColon/header>

// std libraries
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

#if defined HASHCOLON_REAL_PRECISION_DOUBLE
#warning "HashColon is now using double precision floating numbers. Type double is aliased as HashColon::Real"
namespace HashColon
{
    /// @brief Alias for floating numbers: Double precision
    using Real = double;
}
#elif defined HASHCOLON_REAL_PRECISION_SINGLE
#warning "HashColon is now using single precision floating numbers. Type float is aliased as HashColon::Real"
namespace HashColon
{
    /// @brief Alias for floating numbers: Single precision
    using Real = float;
}
#else
#warning "HashColon is now using single precision floating numbers. Type float is aliased as HashColon::Real."
namespace HashColon
{
    /// @brief Alias for floating numbers: Single precision
    using Real = float;
}
#endif

#if __cplusplus >= 202002L
#include <numbers>
#elif __cplusplus >= 201703L
namespace std
{
    namespace numbers
    {
        /// @brief See https://en.cppreference.com/w/cpp/numeric/constants
        template <typename T>
        inline constexpr T e_v = static_cast<T>(exp(1.0));

        /// @brief See https://en.cppreference.com/w/cpp/numeric/constants
        template <typename T>
        inline constexpr T pi_v = static_cast<T>(acos(-1.0));

        /// @brief See https://en.cppreference.com/w/cpp/numeric/constants
        inline constexpr double e = e_v<double>;

        /// @brief See https://en.cppreference.com/w/cpp/numeric/constants
        inline constexpr double pi = pi_v<double>;
    }
}
#else
#error "C++17 or later version is required for HashColon.CPP"
#endif

namespace HashColon::numbers
{
    /// @brief alias for std::numbers::e_v<Real>. See https://en.cppreference.com/w/cpp/numeric/constants
    inline constexpr Real e = std::numbers::e_v<Real>;

    /// @brief alias for std::numbers::pi_v<Real>. See https://en.cppreference.com/w/cpp/numeric/constants
    inline constexpr Real pi = std::numbers::pi_v<Real>;
}

namespace HashColon
{
    /// @brief alias for void pointer
    using VoidPtr = void *;

    /// @brief Types to define a interval with min/max value and a representative value.
    /// @tparam valueTypeT Type of the values, int, float, double & etc
    template <typename valueTypeT>
    struct ValueInterval
    {
        constexpr ValueInterval() : val{0, 0, 0} {};

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

        constexpr valueTypeT &operator[](std::size_t i);
    };

    /// @brief Real with boundaries. Values are defined in range [minVal, maxVal].
    /// @tparam minVal Minimum of the boudnary range.
    /// @tparam maxVal Maximum of the boudnary range.
    /// @tparam isCircular If true, the value is repeated in the range, like angle degrees [0, 360)
    template <const Real &minVal, const Real &maxVal, bool isCircular = false>
    class BoundedReal;

    /// @brief Real with boundaries. Values are defined in range [minVal, maxVal].
    template <const Real &minVal, const Real &maxVal>
    class BoundedReal<minVal, maxVal, false>
    {
    protected:
        Real _val = 0.0;

    public:
        constexpr BoundedReal() = default;
        BoundedReal(const Real &val);
        constexpr operator Real() const;
        BoundedReal &operator=(Real val);
    };

    /// @brief Real which circulates like degrees. Values are defined in range [minVal, maxVal].
    template <const Real &minVal, const Real &maxVal>
    class BoundedReal<minVal, maxVal, true>
    {
    protected:
        Real _val = 0.0;

    public:
        constexpr BoundedReal() = default;
        constexpr BoundedReal(const Real &val);
        constexpr operator Real() const;
        constexpr BoundedReal &operator=(Real val);
    };

    namespace _details
    {
        struct hiddenAnchor
        {
            inline static Real _minDegree = -180.0;
            inline static Real _maxDegree = 180.0;
            inline static Real _negativePI = numbers::pi;
            inline static Real _positivePI = numbers::pi;

            inline static Real _realZero = 0.0;
            inline static Real _realTen = 10.0;
            inline static Real _realMax = std::numeric_limits<Real>::max();
        };
    }

    /// @brief Angle in radians. Circular value [0, pi)
    class Radian;

    /// @brief Angle in degrees. Circular value [0, 360)
    class Degree : public BoundedReal<
                       _details::hiddenAnchor::_minDegree,
                       _details::hiddenAnchor::_maxDegree, true>
    {
    public:
        constexpr Degree() : BoundedReal(){};
        constexpr Degree(const Real &val) : BoundedReal(val){};

        friend constexpr bool operator==(const Degree &lhs, const Degree &rhs);
        friend constexpr bool operator!=(const Degree &lhs, const Degree &rhs);
        constexpr operator Radian() const;
    };

    /// @brief Angle in radians. Circular value [0, pi)
    class Radian : public BoundedReal<
                       _details::hiddenAnchor::_negativePI,
                       _details::hiddenAnchor::_positivePI, true>
    {
    public:
        constexpr Radian() : BoundedReal(){};
        constexpr Radian(const Real &val) : BoundedReal(val){};

        friend constexpr bool operator==(const Radian &lhs, const Radian &rhs);
        friend constexpr bool operator!=(const Radian &lhs, const Radian &rhs);
        constexpr operator Degree() const;
    };
}

// literals for Real
namespace HashColon
{
    // length unit: meter
    Real operator"" _m(long double m);   // meter is basic unit for length
    Real operator"" _mm(long double mm); // millimeter
    Real operator"" _cm(long double cm); // centimeter
    Real operator"" _km(long double km); // kilometer
    Real operator"" _in(long double in); // inch
    Real operator"" _ft(long double ft); // feet
    Real operator"" _mi(long double mi); // mile
    Real operator"" _yd(long double yd); // yard
    Real operator"" _NM(long double NM); // Nautical Mile

    // angle unit
    Radian operator"" _rad(long double rad); // radian
    Degree operator"" _deg(long double deg); // degree

    // weight unit: kilogram
    Real operator"" _kg(long double kg); // kilogram is basic unit for weight
    Real operator"" _mg(long double mg); // milligram
    Real operator"" _g(long double g);   // gram
    Real operator"" _t(long double t);   // ton
    Real operator"" _lb(long double lb); // lb(pound)

    // time unit: second
    Real operator"" _sec(long double sec); // second is basic unit for time
    Real operator"" _min(long double min); // minute
    Real operator"" _hr(long double hr);   // hour
    Real operator"" _day(long double day); // day

    // speed unit: m/s (meter per second)
    Real operator"" _kn(long double kn);     // knots = NM per hour
    Real operator"" _mph(long double mph);   // mph = mile per hour
    Real operator"" _kmph(long double kmph); // kmph = kilometer per hour

    // pressure unit: pascal(Pa) = N/m^2
    Real operator"" _Pa(long double Pa);     // pascal
    Real operator"" _kPa(long double kPa);   // kilo-pascal
    Real operator"" _bar(long double bar);   // bar = 100 kPa
    Real operator"" _atm(long double atm);   // atmosphere
    Real operator"" _psi(long double psi);   // psi = pound per square inch
    Real operator"" _Torr(long double Torr); // Torr

    // Temperature unit: Celsius *** Despite SI unit of temperature is Kelvin, we are using Celsius.
    // Cuz We'll never use Kelvin for sure.
    Real operator"" _degC(long double degC); // Celcius degree
    Real operator"" _degF(long double degF); // Fahrenheight degree
    Real operator"" _Kelvin(long double K);  // Kelvin, (_K is c++ language reserved)
}

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
    using MatrixX4R = Matrix<HashColon::Real, Dynamic, 4>;

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

/* Template & inline functions implementations */
namespace HashColon
{
    template <typename valueTypeT>
    inline constexpr valueTypeT &ValueInterval<valueTypeT>::operator[](std::size_t i)
    {
        return val[i];
    };

    template <const Real &minVal, const Real &maxVal>
    inline BoundedReal<minVal, maxVal, false>::BoundedReal(const Real &val)
    {
        (minVal <= val && maxVal >= val)
            ? _val = val
            : throw std::range_error("BoundedReal out-of-range error");
    }

    template <const Real &minVal, const Real &maxVal>
    inline constexpr BoundedReal<minVal, maxVal, false>::operator Real() const
    {
        return _val;
    };

    template <const Real &minVal, const Real &maxVal>
    inline BoundedReal<minVal, maxVal, false> &BoundedReal<minVal, maxVal, false>::operator=(Real val)
    {
        (minVal <= val && maxVal >= val)
            ? _val = val
            : throw std::range_error("BoundedReal out-of-range error");
        return *this;
    }

    template <const Real &minVal, const Real &maxVal>
    inline constexpr BoundedReal<minVal, maxVal, true>::BoundedReal(const Real &val)
    {
        (minVal <= val && maxVal >= val)
            ? _val = val
            : _val = std::remainder(val - .5 * (minVal + maxVal), maxVal - minVal) +
                     .5 * (minVal + maxVal);
    };

    template <const Real &minVal, const Real &maxVal>
    inline constexpr BoundedReal<minVal, maxVal, true>::operator Real() const
    {
        return _val;
    };

    template <const Real &minVal, const Real &maxVal>
    inline constexpr BoundedReal<minVal, maxVal, true> &BoundedReal<minVal, maxVal, true>::operator=(Real val)
    {
        (minVal <= val && maxVal >= val)
            ? _val = val
            : _val = std::remainder(val - .5 * (minVal + maxVal), maxVal - minVal) +
                     .5 * (minVal + maxVal);
        return *this;
    };

    inline constexpr Degree::operator Radian() const
    {
        return _val * numbers::pi / 180.0;
    }

    inline constexpr bool operator==(const Degree &lhs, const Degree &rhs)
    {
        return (lhs._val == rhs._val) ||
               ((std::fabs(lhs._val) == 180.0) &&
                (std::fabs(lhs._val) == std::fabs(rhs._val)));
    };

    inline constexpr bool operator!=(const Degree &lhs, const Degree &rhs)
    {
        return !(lhs == rhs);
    };

    inline constexpr bool operator==(const Radian &lhs, const Radian &rhs)
    {
        return (lhs._val == rhs._val) ||
               ((std::fabs(lhs._val) == numbers::pi) &&
                (std::fabs(lhs._val) == std::fabs(rhs._val)));
    };

    inline constexpr bool operator!=(const Radian &lhs, const Radian &rhs)
    {
        return !(lhs == rhs);
    };

    inline constexpr Radian::operator Degree() const
    {
        return _val * 180.0 / numbers::pi;
    };

    // length unit: meter
    inline Real operator"" _m(long double m) { return static_cast<Real>(m); }                 // meter is basic unit for length
    inline Real operator"" _mm(long double mm) { return static_cast<Real>(0.001 * mm); }      // millimeter
    inline Real operator"" _cm(long double cm) { return static_cast<Real>(0.01 * cm); }       // centimeter
    inline Real operator"" _km(long double km) { return static_cast<Real>(1000.0 * km); }     // kilometer
    inline Real operator"" _in(long double in) { return static_cast<Real>(in / 39.370); }     // inch
    inline Real operator"" _ft(long double ft) { return static_cast<Real>(ft / 3.2808); }     // feet
    inline Real operator"" _mi(long double mi) { return static_cast<Real>(mi / 0.00062137); } // mile
    inline Real operator"" _yd(long double yd) { return static_cast<Real>(yd / 1.0936); }     // yard
    inline Real operator"" _NM(long double NM) { return static_cast<Real>(1852.0 * NM); }     // Nautical Mile

    // angle unit
    inline Radian operator"" _rad(long double rad) { return Radian(static_cast<Real>(rad)); } // radian
    inline Degree operator"" _deg(long double deg) { return Degree(static_cast<Real>(deg)); } // degree

    // weight unit: kilogram
    inline Real operator"" _kg(long double kg) { return static_cast<Real>(kg); }            // kilogram is basic unit for weight
    inline Real operator"" _mg(long double mg) { return static_cast<Real>(0.000001 * mg); } // milligram
    inline Real operator"" _g(long double g) { return static_cast<Real>(0.001 * g); }       // gram
    inline Real operator"" _t(long double t) { return static_cast<Real>(1000.0 * t); }      // ton
    inline Real operator"" _lb(long double lb) { return static_cast<Real>(lb / 2.2046); }   // lb(pound)

    // time unit: second
    inline Real operator"" _sec(long double sec) { return static_cast<Real>(sec); }                 // second is basic unit for time
    inline Real operator"" _min(long double min) { return static_cast<Real>(60.0 * min); }          // minute
    inline Real operator"" _hr(long double hr) { return static_cast<Real>(3600.0 * hr); }           // hour
    inline Real operator"" _day(long double day) { return static_cast<Real>(24.0 * 3600.0 * day); } // day

    // speed unit: m/s (meter per second)
    inline Real operator"" _kn(long double kn) { return static_cast<Real>(1852.0 / 3600.0 * kn); }        // knots = NM per hour
    inline Real operator"" _mph(long double mph) { return static_cast<Real>(mph / 0.00062137 / 3600.0); } // mph = mile per hour
    inline Real operator"" _kmph(long double kmph) { return static_cast<Real>(1000.0 / 3600.0 * kmph); }  // kmph = kilometer per hour

    // pressure unit: pascal(Pa) = N/m^2
    inline Real operator"" _Pa(long double Pa) { return static_cast<Real>(Pa); }                         // pascal
    inline Real operator"" _kPa(long double kPa) { return static_cast<Real>(1000.0 * kPa); }             // kilo-pascal
    inline Real operator"" _bar(long double bar) { return static_cast<Real>(100.0 * 1000.0 * bar); }     // bar = 100 kPa
    inline Real operator"" _atm(long double atm) { return static_cast<Real>(101.325 * atm); }            // atmosphere
    inline Real operator"" _psi(long double psi) { return static_cast<Real>(6.894757 * 1000.0 * psi); }  // psi = pound per square inch
    inline Real operator"" _Torr(long double Torr) { return static_cast<Real>(101.325 / 760.0 * Torr); } // Torr

    // Temperature unit: Celsius *** Despite SI unit of temperature is Kelvin, we are using Celsius.
    // Cuz We'll never use Kelvin for sure.
    inline Real operator"" _degC(long double degC) { return static_cast<Real>(degC); }                      // Celcius degree
    inline Real operator"" _degF(long double degF) { return static_cast<Real>(5.0 / 9.0 * (degF - 32.0)); } // Fahrenheight degree
    inline Real operator"" _Kelvin(long double K) { return static_cast<Real>(K - 273.15); }                 // Kelvin, (_K is c++ language reserved)
}

#endif
