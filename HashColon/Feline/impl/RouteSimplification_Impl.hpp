#ifndef HASHCOLON_FELINE_ROUTESIMPLIFICATION_IMPL
#define HASHCOLON_FELINE_ROUTESIMPLIFICATION_IMPL

// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <algorithm>
#include <functional>
#include <vector>
// HashColon libraries
#include <HashColon/Real.hpp>
// header file for this source file
#include <HashColon/Feline/RouteSimplification.hpp>

namespace HashColon::Feline
{
	template <typename PointT>
	std::vector<PointT> GetSimplifiedRoute(
		const std::vector<PointT>& waypoints, const SimplifyRouteMethods::RDP params,
		std::function<bool(const PointT&, const PointT&, void*)> validityFunc,
		void* additionals)
	{
		using namespace std;		

		if (waypoints.size() <= 2)
		{
			return waypoints;
		}

		PointT s = waypoints.front(); PointT e = waypoints.back();
		size_t idx = 0;
		Real maxdist = -1.0;
		for (size_t i = 1; i < waypoints.size() - 1; i++)
		{
			PointT m = waypoints[i];

			Position tmps = (Position)s;
			Position tmpe = (Position)e;
			Position tmpm = (Position)m;
			Real dist = abs(CrossTrackDistance(tmpm, tmps, tmpe));

			if (maxdist < dist)
			{
				maxdist = dist;
				idx = i;
			}
		}

		vector<PointT> re; re.clear();
		if (maxdist < params.epsilon && validityFunc(s, e, additionals))
		{
			re.push_back(s); re.push_back(e);
		}
		else
		{
			vector<PointT> fwd; fwd.assign(waypoints.begin(), waypoints.begin() + idx + 1);
			vector<PointT> fwdresult = GetSimplifiedRoute(fwd, params, validityFunc, additionals);

			vector<PointT> aft; aft.assign(waypoints.begin() + idx, waypoints.end());
			vector<PointT> aftresult = GetSimplifiedRoute(aft, params, validityFunc, additionals);

			re.reserve(fwdresult.size() + aftresult.size() - 1);
			re.insert(re.end(), fwdresult.begin(), fwdresult.end());
			re.insert(re.end(), aftresult.begin() + 1, aftresult.end());
		}
		return re;
	}

	namespace _local 
	{
		template <typename PointT>
		size_t DefaultConditionPoint(const std::vector<PointT>& waypoints, size_t s, size_t e)
		{
			size_t idx = 0;
			Real maxdist = -1.0;
			for (size_t i = s + 1; i < e - 1; i++)
			{
				Position tmps = (Position)waypoints[s];
				Position tmpe = (Position)waypoints[e];
				Position tmpp = (Position)waypoints[i];
				Real dist = abs((double)CrossTrackDistance(tmpp, tmps, tmpe));

				if (maxdist < dist)
				{
					maxdist = dist;
					idx = i;
				}
			}
			return idx;
		}

	}

	template <typename PointT>
	void DouglasPeuckerBase<PointT>::RunSimplification(const std::vector<PointT>& waypoints, size_t s, size_t e, std::vector<bool>& result) const
	{
		using namespace std;		

		// ending condition
		if ((e - s) < 2) 
		{ 
			result[s] = result[e] = true;
			return;
		}

		// Get condition point
		size_t p = PickConditionPoint(waypoints, s, e);

		// if condition is met, add condition point to result. Call Douglas-Peucker for sub-trajectories.
		if (Condition(waypoints, s, e, p))
		{
			RunSimplification(waypoints, s, p, result);
			RunSimplification(waypoints, s, e, result);
			result[s] = result[p] = result[e] = true;
		}
		// if condition is not met, return { s, e }
		else 
		{
			result[s] = result[e] = true;			
		}
	}

	template <typename PointT>
	void DouglasPeuckerBase<PointT>::SimplifyRoute(std::vector<PointT>& waypoints) const
	{
		waypoints = GetSimplifiedRoute(waypoints);
	}

	template <typename PointT>
	std::vector<PointT> DouglasPeuckerBase<PointT>::GetSimplifiedRoute(const std::vector<PointT>& waypoints) const
	{
		using namespace std;
		
		vector<bool> result(waypoints.size(), false);
		RunSimplification(waypoints, 0, waypoints.size(), result);
		size_t size = count_if(result.begin(), result.end(), [](const bool& b) { return b; });
		vector<PointT> re; re.reserve(size);
		for (size_t i = 0; i < waypoints.size(); i++) 
		{
			if (result[i]) { re.push_back(waypoints[i]); }
		}
		return re;
	}

	template <typename PointT>
	size_t DouglasPeucker<PointT>::PickConditionPoint(const std::vector<PointT>& waypoints, size_t s, size_t e) const
	{
		return _local::DefaultConditionPoint(waypoints, s, e);
	}

	template <typename PointT>
	bool DouglasPeucker<PointT>::Condition(const std::vector<PointT>& waypoints, size_t s, size_t e, size_t p) const
	{
		Position tmps = (Position)waypoints[s];
		Position tmpe = (Position)waypoints[e];
		Position tmpp = (Position)waypoints[p];
		Real dist = abs(CrossTrackDistance(tmpp, tmps, tmpe));
		return (dist > Epsilon);
	}

	template <typename PointT, typename CondParams>
	size_t ConditionedDouglasPeucker<PointT, CondParams>::PickConditionPoint(
		const std::vector<PointT>& waypoints, size_t s, size_t e) const
	{
		return _local::DefaultConditionPoint(waypoints, s, e);	
	}

	template <typename PointT, typename CondParams>
	bool ConditionedDouglasPeucker<PointT, CondParams>::Condition(
		const std::vector<PointT>& waypoints, size_t s, size_t e, size_t p) const
	{
		return _condition(waypoints, s, e, p, _c);
	}
}

#endif