#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif


#include <cmath>
#include <limits>
#include <Eigen/Eigen>
#include <HashColon/Helper/SingletonCLI.hpp>
#include <HashColon/Feline/TrajectoryClustering/TrajectoryDistanceMeasure.hpp>

using namespace std;
using namespace HashColon::Helper;
using namespace HashColon::Feline::Types;
using namespace HashColon::Feline::TrajectoryClustering::DistanceMeasure;

vector<Simple::XYList> UniformSampling(
	vector<Simple::XYList>& trajlist,
	size_t sampleNumber
)
{
	vector<Simple::XYList> re;
	re.resize(trajlist.size());
	#pragma omp parallel for
	for (int i = 0; i < trajlist.size(); i++)
	{
		re[i] = trajlist[i].GetNormalizedXYList(sampleNumber);
	}
	return re;
}

Real TrajectoryDistanceMeasureBase::Measure(
	const Simple::XYList& a,
	const Simple::XYList& b) const
{
	if (_c.Enable_ReversedSequence)
	{
		return min(
			Measure_core(a, b),
			Measure_core(a.GetReversed(), b)
		);
	}
	else if (!_c.Enable_ReversedSequence)
	{
		return Measure_core(a, b);
	}
	else;
}

Real Hausdorff::Measure_core(
	const Simple::XYList& a,
	const Simple::XYList& b) const
{
	Real maxdist = 0;
#pragma omp parallel for 
	for (size_t i = 0; i < a.size(); i++)
	{
		Real mindist = numeric_limits<HashColon::Helper::Real>::max();
		for (size_t j = 0; j < b.size(); j++)
		{
			HashColon::Helper::Real tmpdist = a[i].DistanceTo(b[i]);
			mindist = mindist > tmpdist ? tmpdist : mindist;
		}
#pragma omp critical
		{
			maxdist = maxdist < mindist ? mindist : maxdist;
		}
	}
	return maxdist;
}

Real Euclidean::Measure_core(
	const Simple::XYList& a,
	const Simple::XYList& b) const
{
	Real dist = 0;
	const size_t& n = a.size() > b.size() ? a.size() : b.size();
	size_t i_a, i_b;

#pragma omp parallel for reduction (+:dist)
	for (size_t i = 0; i < n; i++)
	{
		i_a = i >= a.size() ? a.size() - 1 : i;
		i_b = i >= b.size() ? b.size() - 1 : i;
		dist += a[i_a].DistanceTo(b[i_b]);
	}
	dist /= n;
	return dist;
}

Real Merge::Measure_core(
	const Simple::XYList& a,
	const Simple::XYList& b) const
{
	vector<vector<Real>> A, B;
	A.resize(a.size()); B.resize(a.size());
	for (size_t i = 0; i < a.size(); i++)
	{
		A[i].resize(b.size()); B[i].resize(b.size());
	}

	for (size_t i = 0; i < a.size(); i++)
	{
		for (size_t j = 0; j < b.size(); j++)
		{
			if (i == 0)
				A[i][j] = b.GetDistance(0, j) + a[0].DistanceTo(b[j]);
			else
				A[i][j] = min(
					A[i - 1][j] + a[i - 1].DistanceTo(a[i]),
					B[i - 1][j] + b[j].DistanceTo(a[i]));

			if (j == 0)
				B[i][j] = a.GetDistance(0, i) + b[0].DistanceTo(a[i]);
			else
				B[i][j] = min(
					A[i][j - 1] + a[i].DistanceTo(b[j]),
					B[i][j - 1] + b[j - 1].DistanceTo(b[j]));


		}
	}
	return 2.0 * min(A[a.size() - 1][b.size() - 1], B[a.size() - 1][b.size() - 1])
		/ (a.GetDistance() + b.GetDistance())
		- 1.0;
}

Real LCSS::Measure_core(
	const Simple::XYList& a,
	const Simple::XYList& b) const
{
	assert(a.size() > 1 && b.size() > 1);

	size_t aN = a.size() + 1;
	size_t bN = b.size() + 1;

	// initialize dynamic programming table
	vector<vector<Real>> lcss;
	lcss.resize(aN);
	for (size_t i = 0; i < aN; i++)
		lcss[i].resize(bN);

	for (size_t aend = 0; aend < aN; aend++)
	{
		for (size_t bend = 0; bend < bN; bend++)
		{
			// if any of the index is 0, then 0.0
			if (aend == 0 || bend == 0)
				lcss[aend][bend] = 0;
			// if distance btwn the end points is below Epsilon,
			// end the index difference is less than given delta 
			else if (
				(a[aend].DistanceTo(b[bend]) < _c.Epsilon)
				&& ((aend > bend ? aend - bend : bend - aend) <= _c.Delta))
			{
				lcss[aend][bend] = 1.0 + lcss[aend - 1][bend - 1];
			}
			else
			{
				lcss[aend][bend] = max(lcss[aend - 1][bend], lcss[aend][bend - 1]);
			}
		}
	}

	return lcss[a.size()][b.size()] / ((Real)min(aN, bN));
}

void TrajectoryDistanceMeasureBase::Initialize(const std::string configFilePath)
{
	CLI::App* cli = SingletonCLI::GetInstance().GetCLI("TrajectoryDistanceMeasure");

	if (!configFilePath.empty())
	{
		SingletonCLI::GetInstance().AddConfigFile(configFilePath);
	}

	cli->add_option("--Enable_ReversedSequence", _cDefault.Enable_ReversedSequence,
		"Computes sequence-invariant measure. Computes min(D(A,B), D(A.rev, B))" );	
}

void LCSS::Initialize(const std::string configFilePath)
{
	CLI::App* cli = SingletonCLI::GetInstance().GetCLI("TrajectoryDistanceMeasure.LCSS");

	if (!configFilePath.empty())
	{
		SingletonCLI::GetInstance().AddConfigFile(configFilePath);
	}

	cli->add_option("--Epsilon", _cDefault.Epsilon,
		"Criteria distance between two comparing points in LCSS. Represented as epsilon.");
	cli->add_option("--Delta", _cDefault.Delta,
		"Max index difference between two comparing points in LCSS. Represented as delta.");
	
}

// static member initialization
TrajectoryDistanceMeasureBase::_Params TrajectoryDistanceMeasureBase::_cDefault;
LCSS::_Params LCSS::_cDefault;