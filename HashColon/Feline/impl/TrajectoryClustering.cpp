// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
// dependant external libraries
#include <Eigen/Eigen>
// modified external libraries
#include <CLI11_modified/CLI11.hpp>
#include <CLI11_modified/CLI11_extended.hpp>
// HashColon libraries
#include <HashColon/Real.hpp>
#include <HashColon/SingletonCLI.hpp>
#include <HashColon/Feline/ValueTypes.hpp>
// header file for this source file
#include <HashColon/Feline/TrajectoryClustering.hpp>

using namespace std;
using namespace HashColon;
using namespace HashColon::Feline;

// shared functions
namespace HashColon::Feline::TrajectoryClustering
{
	vector<XYList> UniformSampling(
		vector<XYList>& trajlist,
		size_t sampleNumber
	)
	{
		vector<XYList> re;
		re.resize(trajlist.size());
		#pragma omp parallel for
		for (size_t i = 0; i < trajlist.size(); i++)
		{
			re[i] = trajlist[i].GetNormalizedXYList(sampleNumber);
		}
		return re;
	}

	Real TrajectoryDistanceMeasureBase::Measure(
		const XYList& a,
		const XYList& b) const
	{
		if (_c.Enable_ReversedSequence)
		{
			return min(
				Measure_core(a, b),
				Measure_core(a.GetReversed(), b)
			);
		}
		else
		{
			return Measure_core(a, b);
		}
	}
}

// Measure methods
namespace HashColon::Feline::TrajectoryClustering
{
	Real Hausdorff::Measure_core(
		const XYList& a,
		const XYList& b) const
	{
		Real maxdist = 0;
		#pragma omp parallel for 
		for (size_t i = 0; i < a.size(); i++)
		{
			Real mindist = numeric_limits<Real>::max();
			for (size_t j = 0; j < b.size(); j++)
			{
				Real tmpdist = a[i].DistanceTo(b[i]);
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
		const XYList& a,
		const XYList& b) const
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
		dist /= (Real)n;
		return dist;
	}

	Real Merge::Measure_core(
		const XYList& a,
		const XYList& b) const
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
		const XYList& a,
		const XYList& b) const
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
					&& ((Real)(aend > bend ? aend - bend : bend - aend) <= _c.Delta))
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
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Feline.TrajectoryDistanceMeasure");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--Enable_ReversedSequence", _cDefault.Enable_ReversedSequence,
			"Computes sequence-invariant measure. Computes min(D(A,B), D(A.rev, B))");
	}

	void LCSS::Initialize(const std::string configFilePath)
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Feline.TrajectoryDistanceMeasure.LCSS");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--Epsilon", _cDefault.Epsilon,
			"Criteria distance between two comparing points in LCSS. Represented as epsilon.");
		cli->add_option("--Delta", _cDefault.Delta,
			"Max index difference between two comparing points in LCSS. Represented as delta.");

	}

	void ProjectedPCA::Initialize(const std::string configFilePath)
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Feline.TrajectoryDistanceMeasure.ProjectedPCA");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--PcaDimension", _cDefault.PcaDimension,
			"Cut-Off dimension for PCA. if 0, PCA is not done");

		cli->add_option("--Enable_AutoPca", _cDefault.Enable_AutoPca,
			"Set PCA dimension automatically. if true, PcaDimension is ignored.");
	}

	Eigen::VectorXR GetSingleDimensionVector(const XYList& t)
	{
		Eigen::VectorXR re(t.size() * 2);

		for (size_t i = 0; i < t.size(); i++)
		{
			re(2 * i) = t[i].longitude;
			re(2 * i + 1) = t[i].latitude;
		}
		return re;
	}

	void ProjectedPCA::RunPCA(const vector<XYList>& trajlist)
	{
		using namespace Eigen;
		assert(trajlist.size() > 0);

		size_t vN = trajlist[0].size() * 2;
		size_t sN = trajlist.size();

		// if pca is disabled, ignore.
		if (!_c.Enable_AutoPca && _c.PcaDimension == 0)
		{
			_pca = MatrixXR::Identity(vN, vN);
			return;
		}

		MatrixXR E(vN, vN);
		VectorXR mu(vN);

		//compute E(x*x^T) & E(x)
		for (size_t i = 0; i < trajlist.size(); i++)
		{
			VectorXR x = GetSingleDimensionVector(trajlist[i]);
			assert((size_t)x.size() == vN);
			E = E + x * x.transpose();
			mu = mu + x;
		}
		E /= (Real)sN; mu /= (Real)sN;
		//compute covariance matrix
		MatrixXR cov = E - mu * mu.transpose();

		// compute domain shifting matrix _pca using covariance matrix
		SelfAdjointEigenSolver<MatrixXR> eigenSolver(cov);
		VectorXR lambda = eigenSolver.eigenvalues();

		// if auto option is on, compute maximum difference between lambdas
		if (_c.Enable_AutoPca)
		{
			size_t maxDiffIdx = vN - 1;
			Real maxDiff = 0;
			for (size_t i = vN - 1; i > 0; i--)
			{
				if (maxDiff < (lambda[i] - lambda[i - 1]))
				{
					maxDiff = (lambda[i] - lambda[i - 1]);
					maxDiffIdx = i;
				}
			}
			_c.PcaDimension = vN - maxDiffIdx;
		}
		_pca = eigenSolver.eigenvectors().rightCols(_c.PcaDimension).transpose();
	}

	Real ProjectedPCA::Measure_core(
		const XYList& a, const XYList& b) const
	{
		assert(a.size() == b.size());
		assert((size_t)_pca.cols() == a.size() * 2);

		Eigen::VectorXR avec = GetSingleDimensionVector(a);
		Eigen::VectorXR bvec = GetSingleDimensionVector(b);

		avec = _pca * avec;
		bvec = _pca * bvec;

		return (avec - bvec).norm();
	}

	void ModifiedHausdorff::Initialize(const std::string configFilePath)
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Feline.TrajectoryDistanceMeasure.ModifiedHausdorff");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--NeighborhoodWindowSize", _cDefault.NeighborhoodWindowSize,
			"Size of Neighborhood. w value in paper. 0..1 ");

		cli->add_option("--InlierPortion", _cDefault.InlierPortion,
			"Portion of inliers. alpha value in paper. 0..1 ");
	}

	Real ModifiedHausdorff::Measure_core(
		const XYList& a, const XYList& b) const
	{
		assert(a.size() == b.size());
		size_t N = a.size();
		// compute parameters
		// discretized w: _delta / discretized alpha: _rank 
		int delta = (int)(floor((Real)N * _c.NeighborhoodWindowSize));
		size_t rank = (size_t)(round((Real)N * _c.InlierPortion));
		vector<Real> dista, distb;

		// for each point in a(or b)
		for (size_t i = 0; i < N; i++)
		{
			// compute minimum distance to b(or a) in the neighborhood N(a[i])(or N(b[i]))
			double mina = numeric_limits<Real>::max();
			double minb = numeric_limits<Real>::max();
			for (int d = -delta; d <= delta; d++)
			{
				int j = (int)i + d;
				if (j < 0) continue;
				Real ab = a[i].DistanceTo(b[j]);
				Real ba = b[i].DistanceTo(a[j]);
				mina = mina > ab ? ab : mina;
				minb = minb > ba ? ba : minb;
			}
			dista.push_back(mina);
			distb.push_back(minb);
		}

		// get rank
		sort(dista.begin(), dista.end());
		sort(distb.begin(), distb.end());

		return sqrt(dista[rank] * distb[rank]);
	}

	Real DynamicTimeWarping::Measure_core(
		const XYList& a, const XYList& b) const
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
					warp[i][j] = a[i].DistanceTo(b[j]) + min({
						(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
						(j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
						((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())
						});
			}
		}

		return warp[a.size() - 1][b.size() - 1] / (Real)(a.size() + b.size());
	}

	void Initialize_All_TrajectoryDistanceMeasure()
	{
		TrajectoryDistanceMeasureBase::Initialize();
		LCSS::Initialize();
		ProjectedPCA::Initialize();
		ModifiedHausdorff::Initialize();		
	}
}
