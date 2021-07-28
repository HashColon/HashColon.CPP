#ifndef HASHCOLON_FELINE_TYPES_VOYAGESIMPLE_HPP
#define HASHCOLON_FELINE_TYPES_VOYAGESIMPLE_HPP

#include <HashColon/Feline/Feline_config.h>
#include <array>
#include <vector>
#include <chrono>
#include <limits>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Helper/ClassPropertyForCpp.hpp>
#include <HashColon/Feline/Types/ValueTypes.hpp>

namespace HashColon::Feline
{
	using XY = Position;
	using VVa = Velocity;

	namespace _hidden
	{
		struct _XYT
		{
			union
			{
				struct
				{
					_hidden::_Position _Pos;
				};
				std::array<HashColon::Real, 2> dat;
			};
			TimePoint _TP;
			_XYT() : _Pos(), _TP() {};
		};
		bool operator==(const _XYT& lhs, const _XYT& rhs);
		bool operator!=(const _XYT& lhs, const _XYT& rhs);

		struct _XYVVaT
		{
			union
			{
				struct
				{
					_hidden::_Position _Pos;
					_hidden::_Velocity _Vel;

				};
				std::array<HashColon::Real, 4> dat;
			};
			TimePoint _TP;
			_XYVVaT() : _Pos(), _Vel(), _TP() {};
		};
		bool operator==(const _XYVVaT& lhs, const _XYVVaT& rhs);
		bool operator!=(const _XYVVaT& lhs, const _XYVVaT& rhs);

		struct _XYXtd
		{
			union
			{
				struct
				{
					_hidden::_Position _Pos;
					_hidden::_XTD _Xtd;
				};
				std::array<HashColon::Real, 4> dat;
			};
			_XYXtd() : _Pos(), _Xtd() {};
		};
		bool operator==(const _XYXtd& lhs, const _XYXtd& rhs);
		bool operator!=(const _XYXtd& lhs, const _XYXtd& rhs);
	}

	struct XYT : public _hidden::_XYT
	{
		XYT(const XYT& xyt) : _XYT(xyt) {};
		XYT(const _hidden::_XYT _xyt) : _XYT(_xyt) {};
		XYT() : _XYT() {};
		XYT(HashColon::Real _x, HashColon::Real _y, TimePoint _tp)
		{
			_Pos.longitude = _x;
			_Pos.latitude = _y;
			_TP = _tp;
		};
		XYT(_hidden::_Position _pos, TimePoint _tp)
		{
			_Pos = _pos;
			_TP = _tp;
		};

		XYT& operator=(const XYT& rhs) {
			_Pos = rhs._Pos;
			_TP = rhs._TP;
			return (*this);
		};

		HashColon::Real& operator[](std::size_t i) { return dat[i]; };
		TimePoint GetTimePoint()
		{
			return _TP;
		};

		CREATE_PROPERTY(XYT, public, Position, Pos,
			{ return static_cast<Position>(_Pos); },
			{ _Pos = value; }
		);

		CREATE_PROPERTY(XYT, public, TimePoint, TP,
			{ return _TP; },
			{ _TP = value; }
		);

		HashColon::Real DistanceTo(Position toPoint) const
		{
			Position temp(_Pos);
			return temp.DistanceTo(toPoint);
		};		

		HashColon::Degree AngleTo(Position toPoint) const
		{
			Position temp(_Pos);
			return temp.AngleTo(toPoint);
		};
	};

	struct XYXtd : public _hidden::_XYXtd
	{
		XYXtd(const XYXtd& xyxtd) : _XYXtd(xyxtd) {};
		XYXtd(const _hidden::_XYXtd _xyxtd) : _XYXtd(_xyxtd) {};
		XYXtd() : _XYXtd() {};

		XYXtd(HashColon::Real _x, HashColon::Real _y,
			HashColon::Real _xtdp, HashColon::Real _xtds)
		{
			_Pos.longitude = _x; _Pos.latitude = _y;
			_Xtd.xtdPortside = _xtdp; _Xtd.xtdStarboard = _xtds;
		};

		XYXtd(_hidden::_Position _pos, _hidden::_XTD _xtd)
		{
			_Pos = _pos; _Xtd = _xtd;
		};

		XYXtd& operator=(const XYXtd& rhs)
		{
			_Pos = rhs._Pos; _Xtd = rhs._Xtd;
			return (*this);
		};

		HashColon::Real& operator[](std::size_t i) { return dat[i]; };

		CREATE_PROPERTY(XYXtd, public, Position, Pos,
			{ return static_cast<Position>(_Pos); },
			{ _Pos = value; }
		);
		CREATE_PROPERTY(XYXtd, public, XTD, Xtd,
			{ return static_cast<XTD>(_Xtd); },
			{ _Xtd = value; }
		);

		HashColon::Real DistanceTo(Position toPoint) const
		{
			Position temp(_Pos);
			return temp.DistanceTo(toPoint);
		};

		HashColon::Degree AngleTo(Position toPoint) const
		{
			Position temp(_Pos);
			return temp.AngleTo(toPoint);
		};
	};


	// V, Va are velocity of leg before XY
	// ex) wp2 -> leg(2,3) -> wp3 :
	//     for wp3, XYTVVa contains (X3, Y3, T3, V(leg(2,3)), Va(leg(2,3)))
	struct XYVVaT : public _hidden::_XYVVaT
	{
		XYVVaT(const XYVVaT& xyvvat) : _XYVVaT(xyvvat) {};
		XYVVaT(const _hidden::_XYVVaT _xyvvat) : _XYVVaT(_xyvvat) {};
		XYVVaT() : _XYVVaT() {};
		XYVVaT(
			HashColon::Real _x, HashColon::Real _y,
			HashColon::Real _speed, HashColon::Real _angle,
			TimePoint _tp)
		{
			_Pos.longitude = _x; _Pos.latitude = _y;
			_Vel.speed = _speed; _Vel.angle = _angle;
			_TP = _tp;
		};

		XYVVaT(
			_hidden::_Position _pos,
			_hidden::_Velocity _vel,
			TimePoint _tp)
		{
			_Pos = _pos;
			_Vel = _vel;
			_TP = _tp;
		};

		XYVVaT& operator=(const XYVVaT& rhs)
		{
			_Pos = rhs._Pos;
			_TP = rhs._TP;
			_Vel = rhs._Vel;
			return (*this);
		};

		HashColon::Real& operator[](std::size_t i) { return dat[i]; };
		TimePoint GetTimePoint() const
		{
			return _TP;
		};

		CREATE_PROPERTY(XYVVaT, public, Position, Pos,
			{ return static_cast<Position>(_Pos); },
			{ _Pos = value; }
		);

		CREATE_PROPERTY(XYVVaT, public, TimePoint, TP,
			{ return _TP; },
			{ _TP = value; }
		);

		CREATE_PROPERTY(XYVVaT, public, Velocity, Vel,
			{ return static_cast<Velocity>(_Vel); },
			{ _Vel = value; }
		);

		HashColon::Real DistanceTo(Position toPoint) const
		{
			Position temp(_Pos);
			return temp.DistanceTo(toPoint);
		};

		HashColon::Degree AngleTo(Position toPoint) const
		{
			Position temp(_Pos);
			return temp.AngleTo(toPoint);
		};
	};

	class XYList : public std::vector<XY>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;

		XYList GetNormalizedXYList(size_t sizeN) const;

		std::vector<HashColon::Real> GetParameterized() const;

		XYList GetReversed() const;
	};

	class XYXtdList : public std::vector<XYXtd>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;

		XYXtdList GetNormalizedXYXtdList(size_t sizeN) const;

		XYXtdList GetReversed() const;
		
		XYList ToXYList() const;
	};

	class XYTList : public std::vector<XYT>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		Duration GetDuration(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYTList GetReversed() const;

		XYList ToXYList() const;
	};

	class XYVVaTList : public std::vector<XYVVaT>
	{
	public:
		HashColon::Real GetDistance(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		Duration GetDuration(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYVVaTList GetReversed() const;

		XYList ToXYList() const;
		XYTList ToXYTList() const;
	};
}

#endif