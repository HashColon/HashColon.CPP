// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <cmath>
#include <limits>
// header file for this source file
#include <HashColon/Real.hpp>

using namespace std;

namespace HashColon
{
	_details::hiddenAnchor _anchor;	

	Degree::operator HashColon::Radian() const
	{
		return _val * Constant::PI / 180.0;
	}

	inline bool operator==(const Degree& lhs, const Degree& rhs)
	{
		return (lhs._val == rhs._val) ||
			((std::fabs(lhs._val) == 180.0) &&
				(std::fabs(lhs._val) == std::fabs(rhs._val)));
	};

	inline bool operator!=(const Degree& lhs, const Degree& rhs)
	{
		return !(lhs == rhs);
	};

	inline bool operator==(const Radian& lhs, const Radian& rhs)
	{
		return (lhs._val == rhs._val) ||
			((std::fabs(lhs._val) == Constant::PI) &&
				(std::fabs(lhs._val) == std::fabs(rhs._val)));
	};

	inline bool operator!=(const Radian& lhs, const Radian& rhs)
	{
		return !(lhs == rhs);
	};

	Radian::operator HashColon::Degree() const
	{
		return _val * 180.0 / Constant::PI;
	};
}

// literals for Real
namespace HashColon
{
	// length unit: meter
	long double operator "" _m(long double m) { return m; }					// meter is basic unit for length
	long double operator "" _mm(long double mm) { return 0.001 * mm; }		// millimeter
	long double operator "" _cm(long double cm) { return 0.01 * cm; }		// centimeter
	long double operator "" _km(long double km) { return 1000.0 * km; }		// kilometer
	long double operator "" _in(long double in) { return in / 39.370; }		// inch
	long double operator "" _ft(long double ft) { return ft / 3.2808; }		// feet
	long double operator "" _mi(long double mi) { return mi / 0.00062137; } // mile
	long double operator "" _yd(long double yd) { return yd / 1.0936; }		// yard
	long double operator "" _NM(long double NM) { return 1852 * NM; }		// Nautical Mile

	// angle unit
	Radian operator "" _rad(long double rad) { return Radian((Real)rad); }		// radian
	Degree operator "" _deg(long double deg) { return Degree((Real)deg); }		// degree

	// weight unit: kilogram
	long double operator "" _kg(long double kg) { return kg; }				// kilogram is basic unit for weight
	long double operator "" _mg(long double mg) { return 0.000001 * mg; }	// milligram
	long double operator "" _g(long double g) { return 0.001 * g; }			// gram
	long double operator "" _t(long double t) { return 1000 * t; }			// ton
	long double operator "" _lb(long double lb) { return lb / 2.2046; }		// lb(pound)	

	// time unit: second
	long double operator "" _sec(long double sec) { return sec; }				// second is basic unit for time	
	long double operator "" _min(long double min) { return 60 * min; }			// minute
	long double operator "" _hr(long double hr) { return 3600 * hr; }			// hour
	long double operator "" _day(long double day) { return 24 * 3600 * day; }	// day

	// speed unit: m/s (meter per second)
	long double operator "" _kn(long double kn) { return 1852 / 3600 * kn; }			// knots = NM per hour
	long double operator "" _mph(long double mph) { return mph / 0.00062137 / 3600; }	// mph = mile per hour
	long double operator "" _kmph(long double kmph) { return 1000.0 / 3600 * kmph; }	// kmph = kilometer per hour

	// pressure unit: pascal(Pa) = N/m^2
	long double operator "" _Pa(long double Pa) { return Pa; }							// pascal 
	long double operator "" _kPa(long double kPa) { return 1000 * kPa; }				// kilo-pascal
	long double operator "" _bar(long double bar) { return 100 * 1000 * bar; }			// bar = 100 kPa
	long double operator "" _atm(long double atm) { return 101.325 * atm; }				// atmosphere
	long double operator "" _psi(long double psi) { return 6.894757 * 1000 * psi; }		// psi = pound per square inch
	long double operator "" _Torr(long double Torr) { return 101.325 / 760 * Torr; }	// Torr

	// Temperature unit: Celsius *** Despite SI unit of temperature is Kelvin, we are using Celsius. 
	// Cuz We'll never use Kelvin for sure.
	long double operator "" _degC(long double degC) { return degC; }				// Celcius degree
	long double operator "" _degF(long double degF) { return 5 / 9 * (degF - 32); }	// Fahrenheight degree
	long double operator "" _Kelvin(long double K) { return K - 273.15; }			// Kelvin, (_K is c++ language reserved)
}