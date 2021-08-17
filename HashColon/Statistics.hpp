#ifndef HASHCOLON_STATISTICS_HPP
#define HASHCOLON_STATISTICS_HPP

// std libraries
#include <cassert>
#include <cmath>
#include <memory>
// dependant external libraries
#include <Eigen/Eigen>
// HashColon libraries
#include <HashColon/Real.hpp>

// StatisticalDistributionBase
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

// NormalDistribution
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

// Statistical analysis functions
namespace HashColon::Statistics
{
	struct SimpleStatisticsAnalysisResults
	{
		std::shared_ptr<HashColon::Real> Min;
		std::shared_ptr<HashColon::Real> Max;
		std::shared_ptr<HashColon::Real> Mean;
		std::shared_ptr<HashColon::Real> Median;
		std::shared_ptr<HashColon::Real> Variance;
		std::shared_ptr<HashColon::Real> StdDeviation;

		void NewAll();
	};

	HashColon::Real Min(const std::vector<HashColon::Real>& samples);

	HashColon::Real Min(const std::vector<HashColon::Real>& samples, size_t& idx);

	HashColon::Real Min(const Eigen::MatrixXR& samples);

	HashColon::Real Max(const std::vector<HashColon::Real>& samples);

	HashColon::Real Max(const std::vector<HashColon::Real>& samples, size_t& idx);

	HashColon::Real Max(const Eigen::MatrixXR& samples);

	HashColon::Real Mean(const std::vector<HashColon::Real>& samples);

	HashColon::Real Mean(const Eigen::MatrixXR& samples);

	HashColon::Real Median(const std::vector<HashColon::Real>& samples);

	HashColon::Real Median(const std::vector<HashColon::Real>& samples, size_t& idx);

	HashColon::Real Median(const Eigen::MatrixXR& samples);

	HashColon::Real Variance(const std::vector<HashColon::Real>& samples);

	HashColon::Real Variance(const Eigen::MatrixXR& samples);

	HashColon::Real StdDeviation(const std::vector<HashColon::Real>& samples);

	HashColon::Real StdDeviation(const Eigen::MatrixXR& samples);
}

#endif