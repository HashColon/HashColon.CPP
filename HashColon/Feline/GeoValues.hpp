#ifndef HASHCOLON_FELINE_GEODISTANCES_HPP
#define HASHCOLON_FELINE_GEODISTANCES_HPP

// std libraries
#include <array>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
// HashColon libraries
#include <HashColon/Exception.hpp>
#include <HashColon/Real.hpp>

// GeoValues: ShipIDKey, Position, Velocity, TimePoint, Duration, XTD & combined types
namespace HashColon::Feline
{
	// declare GeoDistanceType in use
	enum GeoDistanceType : size_t
	{
		DefaultDistance, 
		CartesianDistance, 
		HaversineDistance
	};

	// ship id definition
	using ShipIDKey = unsigned int;

	// Position declaration
	struct Position;

	// value bounds for position
	namespace PositionBounds
	{
		constexpr HashColon::Real N = 90.0;
		constexpr HashColon::Real S = 90.0;
		constexpr HashColon::Real E = 180.0;
		constexpr HashColon::Real W = -180.0;
	}	

	// helper functions
	namespace _helper
	{		
		inline bool IsNonNegative(const HashColon::Real& x) { return x >= 0.0; };
		inline bool IsAngle(const HashColon::Real& x) { return x >= -180.0 && x <= 180.0; };
	}

	// position definition
	struct Position
	{		
		union {
			struct
			{
				// do not use boundedReal for lon/lat to use aggregate form
				HashColon::Real longitude;
				HashColon::Real latitude;
			};
			std::array<HashColon::Real, 2> dat;
		};

		// No user-declared constructors for aggregate initialization
		Position() = default;

		inline HashColon::Real& operator[](std::size_t i) { return dat[i]; }
		inline bool IsValid() const;
		inline void Validify();

		HashColon::Real DistanceTo(const Position toPoint, GeoDistanceType type = DefaultDistance) const;
		Position MoveTo(const HashColon::Real distanceMeter, const HashColon::Degree a, GeoDistanceType type = DefaultDistance) const;		
		HashColon::Degree AngleTo(const Position toPoint, GeoDistanceType type = DefaultDistance) const;
		HashColon::Real CrossTrackDistanceTo(const Position path_s, const Position path_e, GeoDistanceType type = DefaultDistance) const;
	};
	inline bool operator==(const Position& lhs, const Position& rhs) { return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1]; };
	inline bool operator!=(const Position& lhs, const Position& rhs) { return !(lhs == rhs); };
	inline bool IsLonValid(const HashColon::Real lon) { return lon >= PositionBounds::W && lon <= PositionBounds::E; };
	inline bool IsLatValid(const HashColon::Real lat) { return lat >= PositionBounds::S && lat <= PositionBounds::N; };
	inline bool IsLonLatValid(const HashColon::Real lon, const HashColon::Real lat) { return IsLonValid && IsLatValid; };
	inline bool IsValid(const Position& pos) { return IsLonLatValid(pos.dat[0], pos.dat[1]); };
	inline bool Position::IsValid() const { return  HashColon::Feline::IsValid(*this); };
	inline void Position::Validify()
	{		
		if (!IsLonValid(longitude))		
			longitude = std::remainder(longitude - 180.0, 360.0) + 180.0;

		if (!IsLatValid(latitude))
			latitude = ((int)std::round(latitude / 180.0) % 2 == 0)
				? std::remainder(latitude - 180.0, 360.0) + 180.0
				: -std::remainder(latitude - 180.0, 360.0);		
	};
	using XY = Position;

	// velocity definition
	struct Velocity
	{
		union {
			struct
			{
				HashColon::Real speed;
				HashColon::Real angle;
			};
			std::array<HashColon::Real, 2> dat;
		};

		// No user-declared constructors for aggregate initialization
		Velocity() = default;

		inline HashColon::Real& operator[](std::size_t i) { return dat[i]; }
		inline bool IsValid() const;
		inline void Validify();
	};
	inline bool operator==(const Velocity& lhs, const Velocity& rhs) { return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1]; };
	inline bool operator!=(const Velocity& lhs, const Velocity& rhs) { return !(lhs == rhs); };
	inline bool IsSpeedValid(const HashColon::Real s) { return _helper::IsNonNegative(s); };
	inline bool IsAngleValid(const HashColon::Real a) { return _helper::IsAngle(a); };
	inline bool IsValid(const Velocity& v) { return IsSpeedValid(v.dat[0]) && IsAngleValid(v.dat[1]); };
	inline bool Velocity::IsValid() const { return HashColon::Feline::IsValid(*this); };
	inline void Velocity::Validify()
	{
		if (!IsSpeedValid(speed))
			speed = std::abs(speed);
		
		if (!IsAngleValid(angle))
			angle = std::remainder(angle - 180.0, 360) + 180.0;		
	}
	using VVa = Velocity;

	// TimeInterval definition as duration
	using Duration = std::chrono::system_clock::duration;

	// TimePoint definition
	class TimePoint : public std::chrono::system_clock::time_point
	{
	public:
		inline static const std::string defaultFormat = "yy-mm-dd HH:MM:SS";

		constexpr TimePoint()
			: std::chrono::system_clock::time_point() {};

		constexpr explicit TimePoint(const Duration& d)
			: std::chrono::system_clock::time_point(d) {};

		template <class Duration2>
		constexpr TimePoint(const time_point<std::chrono::system_clock, Duration2>& t)
			: std::chrono::system_clock::time_point(t) {};

		inline TimePoint(std::string datetimeStr) { fromString(datetimeStr); };
		inline TimePoint(std::pair<std::string, std::string> timedef) { fromString(timedef.first, timedef.second); };

	public:
		inline TimePoint& operator=(std::string datetimeStr) { fromString(datetimeStr); return (*this); };
		inline TimePoint& operator=(std::pair<std::string, std::string> timedef) 
		{ 
			fromString(timedef.first, timedef.second); 
			return (*this); }
		;
		void fromString(std::string datetimeStr, const std::string formatStr = defaultFormat);
		std::string toString(const std::string formatStr = defaultFormat) const;
	};

	// XTD definition
	struct XTD
	{
		union {
			struct
			{
				HashColon::Real xtdPortside;
				HashColon::Real xtdStarboard;
			};
			std::array<HashColon::Real, 2> dat;
		};

		// No user-declared constructors for aggregate initialization
		XTD() = default;

		inline HashColon::Real& operator[](std::size_t i) { return dat[i]; }
		inline bool IsValid() const;
		inline void Validify();
	};
	inline bool operator==(const XTD& lhs, const XTD& rhs) { return lhs.dat[0] == rhs.dat[0] && lhs.dat[1] == rhs.dat[1]; };
	inline bool operator!=(const XTD& lhs, const XTD& rhs) { return !(lhs == rhs); };
	inline bool IsXtdValid(const HashColon::Real x) { return _helper::IsNonNegative(x); };
	inline bool IsValid(const XTD& xtd) { return IsXtdValid(xtd.dat[0]) && IsXtdValid(xtd.dat[1]); };
	inline bool XTD::IsValid() const { return  HashColon::Feline::IsValid(*this); };
	inline void XTD::Validify()
	{
		if (!IsXtdValid(xtdPortside))
			xtdPortside = std::abs(xtdPortside);
		
		if (!IsXtdValid(xtdStarboard))
			xtdStarboard = std::abs(xtdStarboard);
	}

	// XY(Position) + T(TimePoint)
	struct XYT
	{
		Position Pos;
		TimePoint TP;

		// No user-declared constructors for aggregate initialization
		XYT() = default;

		inline HashColon::Real& operator[](std::size_t i) { return Pos.dat[i]; };

		XYT MoveTo(const Velocity vel, const Duration t, GeoDistanceType type = DefaultDistance) const;
		HashColon::Real SpeedTo(const XYT xyt, GeoDistanceType type = DefaultDistance) const;
		Velocity VelocityTo(const XYT xyt, GeoDistanceType type = DefaultDistance) const;
		inline operator Position() const { return Pos; };
		inline operator TimePoint() const { return TP; };
		inline bool IsValid() const;
		inline void Validify();
	};
	inline bool operator==(const XYT& lhs, const XYT& rhs) { return lhs.Pos == rhs.Pos && lhs.TP == rhs.TP; };
	inline bool operator!=(const XYT& lhs, const XYT& rhs) { return !(lhs == rhs); };
	inline bool IsValid(const XYT& xyt) { return xyt.Pos.IsValid(); };
	inline bool XYT::IsValid() const { return HashColon::Feline::IsValid(*this); };
	inline void XYT::Validify() { Pos.Validify(); };

	// XY(Position) + VVa(Velocity) + T(TimePoint)
	/* V, Va are velocity of leg before XY
	 * ex) wp2 -> leg(2,3) -> wp3 :
	 *     for wp3, XYTVVa contains (X3, Y3, T3, V(leg(2,3)), Va(leg(2,3)))
	 */
	struct XYVVaT
	{
		union
		{
			struct
			{
				Position Pos;
				Velocity Vel;
			};
			std::array<HashColon::Real, 4> dat;
		};
		TimePoint TP;

		// No user-declared constructors for aggregate initialization
		XYVVaT() = default;

		inline HashColon::Real& operator[](std::size_t i) { return dat[i]; };

		inline operator Position() const { return Pos; };
		inline operator Velocity() const { return Vel; };
		inline operator TimePoint() const { return TP; };
		inline operator XYT() const { return { Pos, TP }; };

		inline bool IsValid() const;
		inline void Validify();
	};
	inline bool operator==(const XYVVaT& lhs, const XYVVaT& rhs) { return lhs.Pos == rhs.Pos && lhs.TP == rhs.TP && lhs.Vel == rhs.Vel; };
	inline bool operator!=(const XYVVaT& lhs, const XYVVaT& rhs) { return !(lhs == rhs); };
	inline bool IsValid(const XYVVaT& xyvvat) { return xyvvat.Pos.IsValid() && xyvvat.Vel.IsValid(); };
	inline bool XYVVaT::IsValid() const { return HashColon::Feline::IsValid(*this); };
	inline void XYVVaT::Validify() { Pos.Validify(); Vel.Validify(); };

	// XY(Position) + Xtd(XTD)
	struct XYXtd
	{
		union
		{
			struct
			{
				Position Pos;
				XTD Xtd;
			};
			std::array<HashColon::Real, 4> dat;
		};

		// No user-declared constructors for aggregate initialization
		XYXtd() = default;

		inline HashColon::Real& operator[](std::size_t i) { return dat[i]; };

		inline operator Position() const { return Pos; };
		inline operator XTD() const { return Xtd; };

		inline bool IsValid() const;
		inline void Validify();
	};
	inline bool operator==(const XYXtd& lhs, const XYXtd& rhs) { return lhs.Pos == rhs.Pos && lhs.Xtd == rhs.Xtd; };
	inline bool operator!=(const XYXtd& lhs, const XYXtd& rhs) { return !(lhs == rhs); };
	inline bool IsValid(const XYXtd& xyxtd) { return xyxtd.Pos.IsValid() && xyxtd.Xtd.IsValid(); };
	inline bool XYXtd::IsValid() const { return HashColon::Feline::IsValid(*this); };
	inline void XYXtd::Validify() { Pos.Validify(); Xtd.Validify(); };

	// XY(Position) + VVa(Velocity) + Xtd(XTD) + T(TimePoint)
	struct XYVVaXtdT
	{
		union
		{
			struct
			{
				Position Pos;
				Velocity Vel;
				XTD Xtd;
			};
			std::array<HashColon::Real, 6> dat;
		};
		TimePoint TP;

		// No user-declared constructors for aggregate initialization
		XYVVaXtdT() = default;

		inline operator Position() const { return Pos; };
		inline operator Velocity() const { return Vel; };
		inline operator XTD() const { return Xtd; };
		inline operator TimePoint() const { return TP; };
		inline operator XYT() const { return { Pos, TP }; };
		inline operator XYVVaT() const { return { Pos, Vel, TP }; };
		inline operator XYXtd() const { return { Pos, Xtd }; };

		inline bool IsValid() const;
		inline void Validify();
	};
	inline bool operator==(const XYVVaXtdT& lhs, const XYVVaXtdT& rhs)
	{
		return lhs.Pos == rhs.Pos
			&& lhs.Vel == rhs.Vel
			&& lhs.Xtd == rhs.Xtd
			&& lhs.TP == rhs.TP;
	};
	inline bool operator!=(const XYVVaXtdT& lhs, const XYVVaXtdT& rhs) { return !(lhs == rhs); };
	inline bool IsValid(const XYVVaXtdT& xyvvaxtdt)
	{
		return xyvvaxtdt.Pos.IsValid() 
			&& xyvvaxtdt.Vel.IsValid() 
			&& xyvvaxtdt.Xtd.IsValid(); 
	};
	inline bool XYVVaXtdT::IsValid() const { return HashColon::Feline::IsValid(*this); };
	inline void XYVVaXtdT::Validify() { Pos.Validify(); Vel.Validify(); Xtd.Validify(); };
}

// GeoTrajectories: A sequence of GeoValues
namespace HashColon::Feline
{
	class XYList : public std::vector<XY>
	{
	public:
		HashColon::Real GetLength(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		std::vector<HashColon::Real> GetLengths() const;
		XYList GetLengthSampled(const std::vector<HashColon::Real>& lengthParams) const;
		XYList GetUniformLengthSampled(size_t sizeN) const;		
		XYList GetReversed() const;
	};

	class XYXtdList : public std::vector<XYXtd>
	{
	public:		
		HashColon::Real GetLength(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		std::vector<HashColon::Real> GetLengths() const;
		XYXtdList GetLengthSampled(const std::vector<HashColon::Real>& lengthParams) const;
		XYXtdList GetUniformLengthSampled(size_t sizeN) const;		
		XYXtdList GetReversed() const;		
		XYList ToXYList() const;

		inline operator XYList() const { return ToXYList(); };
	};	

	class XYTList : public std::vector<XYT>
	{
	public:		
		HashColon::Real GetLength(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		std::vector<HashColon::Real> GetLengths() const;
		XYTList GetLengthSampled(const std::vector<HashColon::Real>& lengthParams) const;
		XYTList GetUniformLengthSampled(size_t sizeN) const;		
		Duration GetElapsedTime(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		std::vector<Duration> GetElapsedTimes() const;
		XYTList GetTimeSampled(const std::vector<HashColon::Feline::Duration>& timeParams) const;
		XYTList GetUniformTimeSampled(size_t sizeN) const;				
		
		XYTList GetReversed() const;
		XYList ToXYList() const;

		inline operator XYList() const { return ToXYList(); };
	};

	class XYVVaTList : public std::vector<XYVVaT>
	{
	public:		
		HashColon::Real GetLength(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYVVaTList GetLengthSampled(const std::vector<HashColon::Real>& lengthParams) const;
		XYVVaTList GetUniformLengthSampled(size_t sizeN) const;
		std::vector<HashColon::Real> GetLengths() const;
		Duration GetElapsedTime(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		std::vector<Duration> GetElapsedTimes() const;
		XYVVaTList GetTimeSampled(const std::vector<HashColon::Feline::Duration>& timeParams) const;
		XYVVaTList GetUniformTimeSampled(size_t sizeN) const;		

		XYVVaTList GetReversed() const;
		XYList ToXYList() const;
		XYTList ToXYTList() const;

		inline operator XYList() const { return ToXYList(); };
		inline operator XYTList() const { return ToXYTList(); };
	};

	class XYVVaXtdTList : public std::vector<XYVVaXtdT>
	{
	public:		
		HashColon::Real GetLength(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		XYVVaXtdTList GetLengthSampled(const std::vector<HashColon::Real>& lengthParams) const;
		XYVVaXtdTList GetUniformLengthSampled(size_t sizeN) const;
		std::vector<HashColon::Real> GetLengths() const;
		Duration GetElapsedTime(size_t sIndex = 0, size_t eIndex = std::numeric_limits<size_t>::max()) const;
		std::vector<Duration> GetElapsedTimes() const;
		XYVVaXtdTList GetTimeSampled(const std::vector<HashColon::Feline::Duration>& timeParams) const;
		XYVVaXtdTList GetUniformTimeSampled(size_t sizeN) const;		

		XYVVaXtdTList GetReversed() const;
		XYList ToXYList() const;
		XYXtdList ToXYXtdList() const;
		XYTList ToXYTList() const;
		XYVVaTList ToXYVVaTList() const;

		inline operator XYList() const { return ToXYList(); };
		inline operator XYXtdList() const { return ToXYXtdList(); };
		inline operator XYTList() const { return ToXYTList(); };
		inline operator XYVVaTList() const { return ToXYVVaTList(); };
	};
}

// GeoTrajecory with vessel data
namespace HashColon::Feline
{
	struct StaticType
	{
		ShipIDKey imo;
		ShipIDKey mmsi;
		struct {
			HashColon::Real L;
			HashColon::Real B;
			HashColon::Real T;
		} Dim;
	};

	template <typename TList = XYVVaTList>
	struct AisTrajectory
	{
		StaticType staticInfo;
		TList trajectory;
	};
}

// Geospatial distances
namespace HashColon::Feline::CoordSys
{
	class EarthRadius
	{
	private:
		static inline Real _earthRadius = 6371000.0; // earth radius is 6371 km
	public:
		static Real Val() { return _earthRadius; };
		static void Set(Real val) { _earthRadius = val; };
	};

	class GeoCoordSysBase
	{
	public:
		using Ptr = std::shared_ptr<GeoCoordSysBase>;
		virtual HashColon::Real Distance(const Position A, const Position B) = 0;
		virtual HashColon::Degree Angle(const Position A, const Position B) = 0;
		virtual HashColon::Degree Angle(const Position A, const Position B, const Position P) = 0;
		virtual Position MovePoint(const Position A, const HashColon::Real d, const HashColon::Degree alpha) = 0;
		virtual Position MovePoint(const Position A, const Velocity vel, const Duration t) = 0;
		virtual XYT MovePoint(const XYT A, const Velocity vel, const Duration t) = 0;
		virtual HashColon::Real CrossTrackDistance(const Position P, const Position path_S, const Position path_E) = 0;
		virtual HashColon::Real OnTrackDistance(const Position P, const Position path_S, const Position path_E) = 0;
		virtual HashColon::Real Speed(const XYT A, const XYT B) = 0;
		virtual Velocity VelocityBtwn(const XYT A, const XYT B) = 0;
	};

	// computes equirectangular projected distance 
	// ref: https://www.movable-type.co.uk/scripts/latlong.html
	class Cartesian : public GeoCoordSysBase
	{
	private:
		Real _lonUnit = HashColon::Constant::PI / 180.0 * EarthRadius::Val();	// Distance(metre) per degree. This varies to latitude.
		const Real _latUnit = HashColon::Constant::PI / 180.0 * EarthRadius::Val();	// Distance(metre) per degree. This is constant.

	public:
		Real lonUnit() { return _lonUnit; };
		Real lonUnit(const Position base) { return cos(base.latitude * Constant::PI / 180.0) * Constant::PI / 180.0 * EarthRadius::Val(); };
		Real latUnit() { return _latUnit; };
		Real latUnit(const Position base) { return _latUnit; };

	public:
		void SetBase(const Position base) { _lonUnit = lonUnit(base); };

		virtual HashColon::Real Distance(const Position A, const Position B) override;
		virtual HashColon::Degree Angle(const Position A, const Position B) override;
		virtual HashColon::Degree Angle(const Position A, const Position B, const Position P) override;
		virtual Position MovePoint(const Position A, const HashColon::Real d, const HashColon::Degree alpha) override;
		virtual Position MovePoint(const Position A, const Velocity vel, const Duration t) override;
		virtual XYT MovePoint(const XYT A, const Velocity vel, const Duration t)  override;
		virtual HashColon::Real CrossTrackDistance(const Position P, const Position path_S, const Position path_E) override;
		virtual HashColon::Real OnTrackDistance(const Position P, const Position path_S, const Position path_E) override;
		virtual HashColon::Real Speed(const XYT A, const XYT B) override;
		virtual Velocity VelocityBtwn(const XYT A, const XYT B) override;

		// functions with specific base positions
		HashColon::Real Distance_atDesignatedLocation(const Position A, const Position B, const Position base);
		HashColon::Degree Angle_atDesignatedLocation(const Position A, const Position B, const Position base);
		HashColon::Degree Angle_atDesignatedLocation(const Position A, const Position B, const Position P, const Position base);
		Position MovePoint_atDesignatedLocation(const Position A, const HashColon::Real d, const HashColon::Degree alpha, const Position base);
		Position MovePoint_atDesignatedLocation(const Position A, const Velocity vel, const Duration t, const Position base);
		XYT MovePoint_atDesignatedLocation(const XYT A, const Velocity vel, const Duration t, const Position base);
		HashColon::Real CrossTrackDistance_atDesignatedLocation(const Position P, const Position path_S, const Position path_E, const Position base);
		HashColon::Real OnTrackDistance_atDesignatedLocation(const Position P, const Position path_S, const Position path_E, const Position base);
		HashColon::Real Speed_atDesignatedLocation(const XYT A, const XYT B, const Position base);
		Velocity VelocityBtwn_atDesignatedLocation(const XYT A, const XYT B, const Position base);
	};

	// computes Geodesic distances with Haversine/Rhumb line equations.
	// ref: https://www.movable-type.co.uk/scripts/latlong.html
	class Haversine : public GeoCoordSysBase
	{	
		virtual HashColon::Real Distance(const Position A, const Position B) override;
		virtual HashColon::Degree Angle(const Position A, const Position B) override;
		virtual HashColon::Degree Angle(const Position A, const Position B, const Position P) override;
		virtual Position MovePoint(const Position A, const HashColon::Real d, const HashColon::Degree alpha) override;
		virtual Position MovePoint(const Position A, const Velocity vel, const Duration t) override;
		virtual XYT MovePoint(const XYT A, const Velocity vel, const Duration t)  override;
		virtual HashColon::Real CrossTrackDistance(const Position P, const Position path_S, const Position path_E) override;
		virtual HashColon::Real OnTrackDistance(const Position P, const Position path_S, const Position path_E) override;
		virtual HashColon::Real Speed(const XYT A, const XYT B) override;
		virtual Velocity VelocityBtwn(const XYT A, const XYT B) override;
	};
}

//
namespace HashColon::Feline
{
	class GeoDistance
	{		
	public:
		// Method selectable functions
		static HashColon::Real Distance(const Position A, const Position B, GeoDistanceType type = DefaultDistance);
		static HashColon::Degree Angle(const Position A, const Position B, GeoDistanceType type = DefaultDistance);
		static HashColon::Degree Angle(const Position A, const Position B, const Position P, GeoDistanceType type = DefaultDistance);
		static Position MovePoint(const Position A, const HashColon::Real d, const HashColon::Degree alpha, GeoDistanceType type = DefaultDistance);
		static Position MovePoint(const Position A, const Velocity vel, const Duration t, GeoDistanceType type = DefaultDistance);
		static XYT MovePoint(const XYT A, const Velocity vel, const Duration t, GeoDistanceType type = DefaultDistance);
		static HashColon::Real CrossTrackDistance(const Position P, const Position path_S, const Position path_E, GeoDistanceType type = DefaultDistance);
		static HashColon::Real OnTrackDistance(const Position P, const Position path_S, const Position path_E, GeoDistanceType type = DefaultDistance);
		static HashColon::Real Speed(const XYT A, const XYT B, GeoDistanceType type = DefaultDistance);
		static Velocity VelocityBtwn(const XYT A, const XYT B, GeoDistanceType type = DefaultDistance);

		static inline CoordSys::Cartesian cartesian;
		static inline CoordSys::Haversine haversine;
				
	private:		
		/*static inline std::unordered_map<size_t, std::shared_ptr<CoordSys::GeoCoordSysBase>>_init_coordsys()
		{
			std::unordered_map<size_t, std::shared_ptr<CoordSys::GeoCoordSysBase>> re;
			re[DefaultDistance] = nullptr;
			re[CartesianDistance] = std::make_shared<CoordSys::Cartesian>(cartesian);
			re[HaversineDistance] = std::make_shared<CoordSys::Haversine>(haversine);
			return re;
		}*/
		//static inline std::unordered_map<size_t, CoordSys::GeoCoordSysBase::Ptr> _coordsys = _init_coordsys();
		static inline std::unordered_map<size_t, CoordSys::GeoCoordSysBase::Ptr> _coordsys
		{
			{DefaultDistance, nullptr},
			{CartesianDistance, std::make_shared<CoordSys::Cartesian>()},
			{HaversineDistance, std::make_shared<CoordSys::Haversine>()}
			/*{CartesianDistance, std::make_shared<CoordSys::Cartesian>(cartesian)},
			{HaversineDistance, std::make_shared<CoordSys::Haversine>(haversine)}*/
		};

		static inline std::unordered_map<std::string, GeoDistanceType> _coordsysName = 
		{
			{"Cartesian", CartesianDistance},
			{"Haversine", HaversineDistance}
		};

	public:
		// Initialization 
		static void SetBaseLocation(const Position baselocation);
		static void SetDistanceMethod(GeoDistanceType type);
		static void Initialize(GeoDistanceType type, const Position baselocation);
		static void Initialize(const std::string configFilePath = "");

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(GeoDistance);
	};
}


#endif