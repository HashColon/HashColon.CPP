#include <iomanip>
#include <iosfwd>
#include <sstream>
#include <ctime>
#include <mutex>
#include <HashColon/Helper/SimpleVector.hpp>
#include <HashColon/Helper/Real.hpp>
#include <HashColon/Helper/Exception.hpp>
#include <HashColon/Feline/Types/ValueTypes.hpp>

namespace HashColon::Feline::Types
{
	std::mutex ctime_mx;
}

namespace HashColon
{
	namespace Feline
	{
		namespace Types
		{
			using namespace HashColon::Helper;
			
			extern Real _northPole = 90.0;
			extern Real _southPole = -90.0;

			//const Real _LonUnitDist = 88.74 * 1000;  // 88.74km / degree
			//const Real _LatUnitDist = 109.96 * 1000; // 109.96km / degree
			//}
			 
			//{ TimePoint functions
			void TimePoint::fromString(const std::string formatStr, std::string datetimeStr)
			{
				std::tm temp_tm = { 0 };
				std::stringstream ss(datetimeStr.c_str());
				ss >> std::get_time(&temp_tm, formatStr.c_str());

				// check if the datetimestr satisfies the format
				if (ss.fail() || !ss)
					throw std::out_of_range("datetime string out of range ( HashColon::Feline::Types::Timepoint::fromString() ).");
				else
				{
					std::time_t temptime_t;

					// unfortunately, mktime and localtime from ctime is not threadsafe.
					// therefore a lock should be provided.
					// refer to the following link for more information about tregedic behavior of mktime & localtime
					//https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
					{
						std::lock_guard<std::mutex> lock_mx(ctime_mx);
						temptime_t = std::mktime(&temp_tm);
					}												
					auto temp_tp = std::chrono::system_clock::from_time_t(temptime_t);										
					(*this) = temp_tp;
				}
			}

			std::string TimePoint::toString(const std::string formatStr) const
			{				
				std::time_t this_C = std::chrono::system_clock::to_time_t(*this);				
				std::stringstream ss;

				// unfortunately, mktime and localtime from ctime is not threadsafe.
				// therefore a lock should be provided.
				// refer to the following link for more information about tregedic behavior of mktime & localtime
				//https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
				{
					std::lock_guard<std::mutex> lock_mx(ctime_mx);
					ss << std::put_time(std::localtime(&this_C), formatStr.c_str());
				}
				return ss.str();
			}
			//}

			//{ Types::Position functions
			Real Position::DistanceTo_usingCartesianDistance(Position toPoint) const
			{
				{
					using namespace HashColon::Helper::Vec2D;
					std::array<Real, 2> temp;
					temp = minus<Real>(toPoint.dat, dat);
					temp[0] *= _LonUnitDist;
					temp[1] *= _LatUnitDist;
					return abs<Real>(temp);
				}
			}

			Real Position::DistanceTo_usingGrandRoute(Position toPoint) const
			{
				throw NotImplementedException;								
			}

			Position Position::MoveTo_usingCartesianDistance(Degree a, Real distanceMeter) const
			{
				using namespace HashColon::Feline::Types;

				Position re = (*this);

				re.longitude += distanceMeter * sin(a * M_PI / 180) / _LonUnitDist;
				re.latitude += distanceMeter * cos(a * M_PI / 180) / _LatUnitDist;

				return re;
			}

			Position Position::MoveTo_usingGrandRoute(Degree a, Real distanceMeter) const
			{	
				throw NotImplementedException;				
			}

			Degree Position::AngleTo_usingCartesianDistance(Position toPoint) const
			{
				{
					using namespace HashColon::Helper::Vec2D;
					std::array<Real, 2> temp;
					temp = minus<Real>(toPoint.dat, dat);
					temp[0] *= _LonUnitDist;
					temp[1] *= _LatUnitDist;
					Real dist = abs<Real>(temp);

					return (
						(temp[0] >= 0) ? 
						std::acos(temp[1] / dist) : 
						2 * M_PI - std::acos(temp[1] / dist)
						) * 180 / M_PI;
				}
			}

			Degree Position::AngleTo_usingGrandRoute(Position toPoint) const
			{			
				throw NotImplementedException;				
			}

			//}
		}
	}
}



