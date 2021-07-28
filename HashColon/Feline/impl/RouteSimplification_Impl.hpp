#ifndef HASHCOLON_FELINE_ROUTESIMPLIFICATION_IMPL
#define HASHCOLON_FELINE_ROUTESIMPLIFICATION_IMPL

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
		using namespace HashColon::Helper;

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

	template<typename PointT, typename MethodType>
	void SimplifyRoute(
		std::vector<PointT>& waypoints, const MethodType method,
		std::function<bool(const PointT&, const PointT&, void*)> validityFunc,
		void* additionals)
	{
		auto tmp = GetSimplifiedRoute(waypoints, method, validityFunc, additionals);
		waypoints = tmp;
		//waypoints = GetSimplifiedRoute(waypoints, method, validityFunc, additionals);
	}
}

#endif