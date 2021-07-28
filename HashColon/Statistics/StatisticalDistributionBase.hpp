#ifndef HASHCOLON_STATISTICS_STATISTICALDISTRIBUTIONBASE_HPP
#define HASHCOLON_STATISTICS_STATISTICALDISTRIBUTIONBASE_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <cassert>
#include <Eigen/Eigen>
#include <HashColon/Core/Real.hpp>

namespace HashColon::Statistics
{
	template <int dimension>
	class StatisticalDistributionBase
	{
	protected:
		StatisticalDistributionBase() { assert(dimension > 0 || dimension == -1); };
	private:
		virtual HashColon::Real PDF(const Eigen::VectorR<dimension>& x) = 0;		
		virtual Eigen::VectorR<dimension> Mean() = 0;
		virtual Eigen::VectorR<dimension> Mode() = 0;
		virtual Eigen::MatrixR<dimension> Variance() = 0;
		virtual HashColon::Real Entropy() = 0;
	};

	template <>
	class StatisticalDistributionBase<1>
	{
	protected:
		StatisticalDistributionBase() {};
	private:
		virtual HashColon::Real PDF(HashColon::Real x) = 0;
		virtual HashColon::Real CDF(HashColon::Real x) = 0;
		virtual HashColon::Real Mean() = 0;
		virtual HashColon::Real Mode() = 0;
		virtual HashColon::Real Variance() = 0;
		virtual HashColon::Real Entropy() = 0;
	};
}

#endif