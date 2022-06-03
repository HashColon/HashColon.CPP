#ifndef HASHCOLON_FELINE_ROUTESIMPLIFICATION
#define HASHCOLON_FELINE_ROUTESIMPLIFICATION

// std libraries
#include <functional>
#include <vector>
// HashColon libraries
#include <HashColon/Real.hpp>
#include <HashColon/GeoValues.hpp>

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
		const std::vector<PointT> &waypoints, const SimplifyRouteMethods::RDP params,
		std::function<bool(const PointT &, const PointT &, void *)> validityFunc,
		void *additionals);

	/// <summary>
	/// Route simplification. Reduce number of points in given waypoints. Input route is modified.
	/// </summary>
	/// <typeparam name="PointT">Type of point</typeparam>
	/// <typeparam name="MethodType">Simplification method and its parameters. Defined in Felin::SimplifyRouteMethods</typeparam>
	/// <param name="waypoints">waypoint list</param>
	/// <param name="method">method with params</param>
	/// <param name="validityFunc">Checks the validity of a path between two points. The simplified route should only have valid paths.</param>
	template <typename PointT, typename MethodType>
	void SimplifyRoute(
		std::vector<PointT> &waypoints, const MethodType method,
		std::function<bool(const PointT &, const PointT &, void *)> validityFunc,
		void *additionals);

	template <typename PointT>
	class DouglasPeuckerBase
	{
	protected:
		void RunSimplification(const std::vector<PointT> &waypoints, size_t s, size_t e, std::vector<bool> &result) const;
		virtual size_t PickConditionPoint(const std::vector<PointT> &waypoints, size_t s, size_t e) const = 0;
		virtual bool Condition(const std::vector<PointT> &waypoints, size_t s, size_t e, size_t p) const = 0;

	public:
		std::vector<PointT> GetSimplifiedRoute(const std::vector<PointT> &waypoints) const;
		void SimplifyRoute(std::vector<PointT> &waypoints) const;
	};

	template <typename PointT>
	class DouglasPeucker : public DouglasPeuckerBase<PointT>
	{
	protected:
		HashColon::Real Epsilon;
		virtual size_t PickConditionPoint(const std::vector<PointT> &waypoints, size_t s, size_t e) const override;
		virtual bool Condition(const std::vector<PointT> &waypoints, size_t s, size_t e, size_t p) const override;

	public:
		DouglasPeucker(HashColon::Real epsilon) : DouglasPeuckerBase<PointT>(), Epsilon(epsilon){};
	};

	template <typename PointT, typename CondParams>
	class ConditionedDouglasPeucker : public DouglasPeuckerBase<PointT>
	{
	public:
		using CondFunc = std::function<bool(const std::vector<PointT> &, size_t, size_t, size_t, const CondParams &)>;

	protected:
		CondFunc _condition;
		CondParams _c;
		virtual size_t PickConditionPoint(const std::vector<PointT> &waypoints, size_t s, size_t e) const override;
		virtual bool Condition(const std::vector<PointT> &waypoints, size_t s, size_t e, size_t p) const override;

	public:
		ConditionedDouglasPeucker(CondFunc condFunc, CondParams params)
			: DouglasPeuckerBase<PointT>(),
			  _condition(condFunc), _c(params){};
	};

}

#endif

#include <HashColon/Feline/impl/RouteSimplification_Impl.hpp>