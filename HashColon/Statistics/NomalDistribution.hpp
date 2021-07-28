#ifndef HASHCOLON_STATISTICS_NORMALDISTRIBUTION_HPP
#define HASHCOLON_STATISTICS_NORMALDISTRIBUTION_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <cmath>
#include <Eigen/Eigen>

#include <HashColon/Core/Real.hpp>
#include <HashColon/Statistics/StatisticalDistributionBase.hpp>

namespace HashColon::Statistics
{
	template <int dimension = 1> //do not set dimension as size_t. cuz -1 value is used for Eigen::dynamic
	class NormalDistribution : public StatisticalDistributionBase<dimension>
	{
	protected:
		Eigen::VectorR<dimension> _mu;
		Eigen::MatrixR<dimension> _Sigma;

	public:
		// initialize with mu as 0, Sigma as Identity
		NormalDistribution() :
			StatisticalDistributionBase<dimension>(),
			_mu(Eigen::VectorR<dimension>::Zero()),
			_Sigma(Eigen::MatrixR<dimension>::Identity())
		{};

		// initialize with given mu and Sigma as Identity
		NormalDistribution(const Eigen::VectorR<dimension>& mu) :
			StatisticalDistributionBase<dimension>(),
			_mu(mu), _Sigma(Eigen::MatrixR<dimension>::Identity())
		{};

		// initialize with mu = 0 and given Sigma
		NormalDistribution(const Eigen::MatrixR<dimension>& Sigma) :
			StatisticalDistributionBase<dimension>(),
			_mu(Eigen::VectorR<dimension>::Zero()), _Sigma(Sigma)
		{		
			assert(Sigma.rows() == Sigma.cols());
			assert(Sigma.isInvertible());
		};

		// initialize with given mu and Sigma
		NormalDistribution(
			const Eigen::VectorR<dimension>& mu,
			const Eigen::MatrixR<dimension>& Sigma) :
			StatisticalDistributionBase<dimension>(),
			_mu(mu), _Sigma(Sigma)		
		{		
			assert(Sigma.rows() == Sigma.cols());
			assert(Sigma.determinant() != 0);			
		};

		HashColon::Real PDF(const Eigen::VectorR<dimension>& x) override
		{
			using namespace std;
			return pow(2 * M_PI, -0.5 * dimension) * pow(_Sigma.determinant(), -0.5)
				* exp(-0.5 * (x - _mu).transpose() * _Sigma.inverse() * (x - _mu));
		};

		Eigen::VectorR<dimension> Mean() override { return _mu; };

		Eigen::VectorR<dimension> Mode() override { return _mu; };

		Eigen::MatrixR<dimension> Variance() override { return _Sigma; };

		HashColon::Real Entropy() override {
			using namespace std;
			return 0.5 * log(
				(2 * M_PI * M_E * _Sigma).determinant()
			);
		};
		
	};

	template<>
	class NormalDistribution<1> : public StatisticalDistributionBase<1>
	{
	protected:
		HashColon::Real _mu;
		HashColon::Real _sigma;
	public:	
		// initialize with given mu and Sigma
		NormalDistribution(
			HashColon::Real mu = 0,
			HashColon::Real sigma = 1) 
			: StatisticalDistributionBase<1>()
		{
			assert(sigma >= 0);
		};

		HashColon::Real PDF(HashColon::Real x) override;
		HashColon::Real CDF(HashColon::Real x) override;
		HashColon::Real Mean() override;
		HashColon::Real Mode() override;
		HashColon::Real Variance() override;
		HashColon::Real Entropy() override;
	};

	using BivariateNormalDistribution = NormalDistribution<2>;
	using BVN = NormalDistribution<2>;
		
	template <int dimension>
	using GaussianDistribution = NormalDistribution<dimension>;
}

#endif
