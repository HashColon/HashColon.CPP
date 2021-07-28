#ifndef HASHCOLON_FELINE_ROUTESIMPLIFICATION_HPP
#define HASHCOLON_FELINE_ROUTESIMPLIFICATION_HPP

#include <functional>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Feline/Types/ValueTypes.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>

namespace HashColon::Feline
{
	namespace SimplifyRouteMethods
	{
		struct RDP
		{
			HashColon::Real epsilon;
		};
	}

	/// <summary>
	/// Route simplification. Reduce number of points in given waypoints;
	/// </summary>
	/// <typeparam name="PointT">Type of point</typeparam>	
	/// <param name="waypoints">waypoint list</param>
	/// <param name="method">method with params</param>
	/// <param name="validityFunc">Checks the validity of a path between two points. The simplified route should only have valid paths.</param>
	/// <returns>Simplified route</returns>
	template <typename PointT>
	std::vector<PointT> GetSimplifiedRoute(
		const std::vector<PointT>& waypoints, const SimplifyRouteMethods::RDP params,
		std::function<bool(const PointT&, const PointT&, void*)> validityFunc,
		void* additionals);
			
	/// <summary>
	/// Route simplification. Reduce number of points in given waypoints. Input route is modified.
	/// </summary>
	/// <typeparam name="PointT">Type of point</typeparam>
	/// <typeparam name="MethodType">Simplification method and its parameters. Defined in Felin::SimplifyRouteMethods</typeparam>
	/// <param name="waypoints">waypoint list</param>
	/// <param name="method">method with params</param>
	/// <param name="validityFunc">Checks the validity of a path between two points. The simplified route should only have valid paths.</param>
	template<typename PointT, typename MethodType>
	void SimplifyRoute(
		std::vector<PointT>& waypoints, const MethodType method,
		std::function<bool(const PointT&, const PointT&, void*)> validityFunc,
		void* additionals);
}

#endif

#include <HashColon/Feline/impl/RouteSimplification_Impl.hpp>