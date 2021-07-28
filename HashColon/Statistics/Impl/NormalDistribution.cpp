#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <cmath>
#include <Eigen/Eigen>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Statistics/NomalDistribution.hpp>

using namespace std;
using namespace Eigen;
using namespace HashColon;
using namespace HashColon::Statistics;

Real NormalDistribution<1>::PDF(Real x)
{
	return 1.0 / (_sigma * sqrt(2 * M_PI)) * exp(-0.5 * pow((x - _mu) / _sigma, 2));
}

Real NormalDistribution<1>::CDF(Real x)
{
	return 0.5 * (
		1 + erf((x-_mu) / (_sigma * sqrt(2)))
		);
}

Real NormalDistribution<1>::Mean() { return _mu; }

Real NormalDistribution<1>::Mode() { return _mu; }

Real NormalDistribution<1>::Variance() { return _sigma * _sigma; }

Real NormalDistribution<1>::Entropy()
{
	return 0.5 * log(2 * M_PI * M_E * _sigma * _sigma);
}
