#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <algorithm>
#include <cmath>
#include <limits>
#include <Eigen/Eigen>
#include <HashColon/Core/SingletonCLI.hpp>
#include <HashColon/Feline/TrajectoryClustering/XtdDistance.hpp>
#include <HashColon/Feline/TrajectoryClustering/TrajectoryDistanceMeasure_withXTD.hpp>

using namespace std;
using namespace HashColon;
using namespace HashColon::Feline;

mutex _m;

namespace HashColon::Feline::XtdTrajectoryClustering
{
	vector<XYXtdList> UniformSampling(
		vector<XYXtdList>& trajlist, size_t sampleNumber)
	{
		vector<XYXtdList> re;
		re.resize(trajlist.size());
#pragma omp parallel for
		for (size_t i = 0; i < trajlist.size(); i++)
		{
			re[i] = trajlist[i].GetNormalizedXYXtdList(sampleNumber);
		}
		return re;
	}

	Real XtdTrajectoryDistanceMeasureBase::Measure(
		const XYXtdList& a,
		const XYXtdList& b) const
	{
		Real re;
		if (_c.Enable_ReversedSequence)
		{
			re = min(
				Measure_core(a, b),
				Measure_core(a.GetReversed(), b)
			);
		}
		else
		{
			re = Measure_core(a, b);
		}
		assert(re >= 0);
		assert(!isnan(re));
		return re;
	}

	void XtdTrajectoryDistanceMeasureBase::Initialize(const string configFilePath)
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Feline.XtdTrajectoryDistanceMeasure");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--Enable_ReversedSequence", _cDefault.Enable_ReversedSequence,
			"Computes sequence-invariant measure. Computes min(D(A,B), D(A.rev, B))");	}

	Real DtwXtd::Measure_core(
		const XYXtdList& a, const XYXtdList& b) const
	{
		vector<vector<Real>> warp;

		// initialize arrays
		warp.resize(a.size());
		for (size_t i = 0; i < a.size(); i++)
			warp[i].resize(b.size());
				
		for (size_t i = 0; i < a.size(); i++)
		{
			for (size_t j = 0; j < b.size(); j++)
			{
				if (i == 0 && j == 0)
					warp[i][j] = 0;
				else
				{	
					warp[i][j] = a[i].DistanceTo(b[j]._Pos)
						+ min({
							(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
							(j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
							((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())
							});

					assert(!isnan(warp[i][j]));
					assert(warp[i][j] >= 0);
				}
			}
		}

		assert(warp[a.size() - 1][b.size() - 1] >= 0);
		return warp[a.size() - 1][b.size() - 1] / (Real)(a.size() + b.size());
	}

	void DtwXtd_usingJSDivergence::Initialize(const string configFilePath)
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Feline.XtdTrajectoryDistanceMeasure.DtwXtd_JS");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--MonteCarloDomainUnit", _cDefault.MonteCarloDomainUnit,
			"Interval of Monte Carlo integration samples in the unit of sigma.");

		cli->add_option("--MonteCarloDomainSize", _cDefault.MonteCarloDomainSize,
			"Size of sampling range of  Monte Carlo integration samples in the unit of sigma.");

		cli->add_option("--MonteCarloErrorEpsilon", _cDefault.MonteCarloErrorEpsilon,
			"Error threshold for Monte Carlo integration.");
	}

	Real DtwXtd_usingJSDivergence::Measure_core(
		const XYXtdList& a, const XYXtdList& b) const
	{
		vector<vector<Real>> warp;

		// initialize arrays
		warp.resize(a.size());
		for (size_t i = 0; i < a.size(); i++)
			warp[i].resize(b.size());

		for (size_t i = 0; i < a.size(); i++)
		{
			for (size_t j = 0; j < b.size(); j++)
			{
				if (i == 0 && j == 0)
					warp[i][j] = 0;
				else
				{
					Degree aDir, bDir;
					aDir = (i == (a.size() - 1)) ? a[i - 1].AngleTo(a[i]._Pos) : a[i].AngleTo(a[i + 1]._Pos);
					bDir = (j == (b.size() - 1)) ? b[j - 1].AngleTo(b[j]._Pos) : b[j].AngleTo(b[j + 1]._Pos);

					warp[i][j] =
						JSDivergenceDistance(a[i], aDir, b[j], bDir,
							{ _c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon })
						+ min({
						(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
						(j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
						((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())
							});
					
					assert(!isnan(warp[i][j]));
					assert(warp[i][j] >= 0);
				}
			}
		}
		assert(warp[a.size() - 1][b.size() - 1] >= 0);
		return warp[a.size() - 1][b.size() - 1] / (Real)(a.size() + b.size());
	}

	void DtwXtd_usingWasserstein::Initialize(const string configFilePath)
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Feline.XtdTrajectoryDistanceMeasure.DtwXtd_EMD");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--MonteCarloDomainUnit", _cDefault.MonteCarloDomainUnit,
			"Interval of Monte Carlo integration samples in the unit of sigma.");

		cli->add_option("--MonteCarloDomainSize", _cDefault.MonteCarloDomainSize,
			"Size of sampling range of  Monte Carlo integration samples in the unit of sigma.");

		cli->add_option("--MonteCarloErrorEpsilon", _cDefault.MonteCarloErrorEpsilon,
			"Error threshold for Monte Carlo integration.");
	}


	Real DtwXtd_usingWasserstein::Measure_core(
		const XYXtdList& a, const XYXtdList& b) const
	{
		vector<vector<Real>> warp;

		// initialize arrays
		warp.resize(a.size());
		for (size_t i = 0; i < a.size(); i++)
			warp[i].resize(b.size());

		for (size_t i = 0; i < a.size(); i++)
		{
			for (size_t j = 0; j < b.size(); j++)
			{
				if (i == 0 && j == 0)
					warp[i][j] = 0;
				else
				{
					Degree aDir, bDir;
					aDir = (i == (a.size() - 1)) ? a[i - 1].AngleTo(a[i]._Pos) : a[i].AngleTo(a[i + 1]._Pos);
					bDir = (j == (b.size() - 1)) ? b[j - 1].AngleTo(b[j]._Pos) : b[j].AngleTo(b[j + 1]._Pos);

					warp[i][j] =
						WassersteinDistance(a[i], aDir, b[j], bDir,
							{ _c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon })
						+ min({
						(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
						(j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
						((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())
							});

					assert(!isnan(warp[i][j]));
					assert(warp[i][j] >= 0);
				}
			}
		}
		assert(warp[a.size() - 1][b.size() - 1] >= 0);
		return warp[a.size() - 1][b.size() - 1] / (Real)(a.size() + b.size());
	}

	void DtwXtd_BlendedDistance::Initialize(const string configFilePath)
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Feline.XtdTrajectoryDistanceMeasure.DtwXtd_Blended");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--MonteCarloDomainUnit", _cDefault.MonteCarloDomainUnit,
			"Interval of Monte Carlo integration samples in the unit of sigma.");

		cli->add_option("--MonteCarloDomainSize", _cDefault.MonteCarloDomainSize,
			"Size of sampling range of  Monte Carlo integration samples in the unit of sigma.");

		cli->add_option("--MonteCarloErrorEpsilon", _cDefault.MonteCarloErrorEpsilon,
			"Error threshold for Monte Carlo integration.");

		cli->add_option("--PfXtdSigmaRatio", _cDefault.Pf_XtdSigmaRatio,
			"Ratio of PF distance sigma and XTD.");

		cli->add_option("--Coeff_Euclidean", _cDefault.Coeff_Euclidean,
			"Blend coefficient for Euclidean distance.");

		cli->add_option("--Coeff_JS", _cDefault.Coeff_JS,
			"Blend coefficient for JS divergence.");

		cli->add_option("--Coeff_EMD", _cDefault.Coeff_WS,
			"Blend coefficent for Wasserstein distance(Earth-Moving distance).");

		cli->add_option("--Coeff_PF", _cDefault.Coeff_PF,
			"Blend coefficient for PF distance.");
	}

	Real DtwXtd_BlendedDistance::Measure_core(
		const XYXtdList& a, const XYXtdList& b) const
	{		
		vector<vector<Real>> warp;

		// initialize arrays
		warp.resize(a.size());
		for (size_t i = 0; i < a.size(); i++)
			warp[i].resize(b.size());

		for (size_t i = 0; i < a.size(); i++)
		{
			for (size_t j = 0; j < b.size(); j++)
			{
				if (i == 0 && j == 0)
					warp[i][j] = 0;
				else
				{
					Degree aDir, bDir;
					aDir = (i == (a.size() - 1)) ? a[i - 1].AngleTo(a[i]._Pos) : a[i].AngleTo(a[i + 1]._Pos);
					bDir = (j == (b.size() - 1)) ? b[j - 1].AngleTo(b[j]._Pos) : b[j].AngleTo(b[j + 1]._Pos);

					// debug
					/*Real D_Euclidean = a[i].DistanceTo(b[j]._Pos);
					Real D_JS = JSDivergenceDistance(a[i], aDir, b[j], bDir,
						{ _c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon });
					Real D_PF = PFDistance(a[i], aDir, b[j], bDir, { _c.Pf_XtdSigmaRatio });
					Real D_WS = WassersteinDistance(a[i], aDir, b[j], bDir,
						{ _c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon });*/

					warp[i][j] =
						(_c.Coeff_Euclidean > 0 ? _c.Coeff_Euclidean * a[i].DistanceTo(b[j]._Pos) : 0 ) +
						(_c.Coeff_JS > 0 ? _c.Coeff_JS * JSDivergenceDistance(a[i], aDir, b[j], bDir,
							{ _c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon }) : 0) +
						(_c.Coeff_PF > 0 ? _c.Coeff_PF * PFDistance(a[i], aDir, b[j], bDir, { _c.Pf_XtdSigmaRatio }): 0) +
						(_c.Coeff_WS > 0 ? _c.Coeff_WS * WassersteinDistance(a[i], aDir, b[j], bDir,
							{ _c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon }) : 0) +
						+ min({
						(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
						(j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
						((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())
							});

					assert(!isnan(warp[i][j]));
					assert(warp[i][j] >= 0);
				}
			}
		}
		
		assert(warp[a.size() - 1][b.size() - 1] >= 0);
		return warp[a.size() - 1][b.size() - 1] / (Real)(a.size() + b.size());
	}

	void Initialize_All_XtdTrajectoryDistanceMeasure()
	{
		XtdTrajectoryDistanceMeasureBase::Initialize();
				
		DtwXtd_usingJSDivergence::Initialize();
		DtwXtd_usingWasserstein::Initialize();
		DtwXtd_BlendedDistance::Initialize();
	}
}