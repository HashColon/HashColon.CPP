#ifndef HASHCOLON_STATISTICS_SIMPLESTATISTICALANALYSIS_HPP
#define HASHCOLON_STATISTICS_SIMPLESTATISTICALANALYSIS_HPP

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <memory>
#include <vector>
#include <Eigen/Eigen>
#include <HashColon/Core/Real.hpp>

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

