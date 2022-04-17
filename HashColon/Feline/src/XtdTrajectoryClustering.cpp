// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
// dependant external libraries
#include <Eigen/Eigen>
// modified external libraries
#include <HashColon/CLI11.hpp>
#include <HashColon/CLI11_JsonSupport.hpp>
#include <Wasserstein/Wasserstein.hh>
// HashColon libraries
#include <HashColon/Real.hpp>
#include <HashColon/SingletonCLI.hpp>
#include <HashColon/Statistics.hpp>
#include <HashColon/Feline/GeoValues.hpp>
// header file for this source file
#include <HashColon/Feline/XtdTrajectoryClustering.hpp>

#include <iostream>

using namespace std;
using namespace Eigen;
using namespace HashColon;
using namespace HashColon::Feline;

// XTD distance btwn waypoints
namespace HashColon::Feline::XtdTrajectoryClustering
{
	mutex _XtdDistance_cpp_mutex;

	namespace _hidden
	{
		static unique_ptr<vector<vector<Real>>> zBvn;
		static Real zUnit;
		static Real zDomain;

		struct SampleType
		{
			array<Real, 2> Pos;
			Real ProbValue;
		};

		vector<vector<Real>> PreComputeZNormals(Real stepSize, Real domainSize)
		{
			int k = (int)floor(domainSize / stepSize);
			Statistics::BVN bvn;
			vector<vector<Real>> re(2 * k + 1, vector<Real>(2 * k + 1));

			for (int i = -k; i <= k; i++)
				for (int j = -k; j <= k; j++)
				{
					Eigen::Vector2R tmp;
					tmp << (Real)i * stepSize, (Real)j * stepSize;
					re[i + k][j + k] = bvn.PDF(tmp);
				}
			return re;
		}

		/* Computes sampled points in domain. Result samples are not ordered.
		 * The return vectors are in distance space, which means the units are in meters,
		 * not in lat/lon space.
		 * Projection of uniform sampling from symmetric normal distribution for each quadrant
		 * ex)
		 *            +---------+----+    each quardrant is distored quadrant
		 *	avg(xtds, |         |    |    of symmetric normal distribution.
		 *      xtdp) |         |    |    uniform sampled points are projected
		 *            +---------+----+    to the distorted quardrants
		 *  avg(xtds, |         |    |
		 *      xtdp) |         |    |
		 *            +---------+----+
		 *             xtdp       xtds
		 */
		vector<SampleType> GetBVNSamples(
			XYXtd point, Degree direction,
			Real stepSize,						  // sample rate in unit of sigma
			Real domainSize,					  // size of the domain box in unit of sigma
			shared_ptr<Real> pdfTotVal = nullptr) // nomalize Pdf so that the sum of pdf satisfies sum-to-one property
		{
			assert(domainSize > 0);
			assert(stepSize > 0);

			// Get base vectors
			Position pos = point.Pos;
			Degree aS = direction + 90; // starboard direction
			Degree aH = direction;
			Real sigmaP = point.Xtd.xtdPortside / domainSize;  // set sigma in portside direction as 1/3 of xtd portside
			Real sigmaS = point.Xtd.xtdStarboard / domainSize; // set sigma in starboard direction  as 1/3 of xtd starboard
			Real sigmaH = (sigmaP + sigmaS) / 2;			   // set sigma in fwd/back direction as average of p/s sigma1
			int k = (int)floor(domainSize / stepSize);
			size_t N = (2 * k + 1) * (2 * k + 1);

			if (zDomain != domainSize || zUnit != stepSize || !zBvn)
			{
				lock_guard<mutex> _lg(_XtdDistance_cpp_mutex);
				zDomain = domainSize;
				zUnit = stepSize;
				zBvn = make_unique<vector<vector<Real>>>(PreComputeZNormals(zUnit, zDomain));
			}

			vector<SampleType> re;
			re.reserve(N);
			Real totProbVal = 0.0;
			for (int i = -k; i <= k; i++)
			{
				for (int j = -k; j <= k; j++)
				{
					Real x = i <= 0 ? sigmaP * stepSize * i : sigmaS * stepSize * i;
					Real y = sigmaH * stepSize * j;
					Position tmpPos = pos.MoveTo(x, aS).MoveTo(y, aH);
					SampleType tmp;
					tmp.Pos[0] = tmpPos.dat[0];
					tmp.Pos[1] = tmpPos.dat[1];
					{
						lock_guard<mutex> _lg(_XtdDistance_cpp_mutex);
						tmp.ProbValue = (*zBvn)[i + k][j + k];
						totProbVal += (*zBvn)[i + k][j + k];
					}
					// cout << i << ", " << j << endl;
					re.push_back(tmp);
				}
			}

			for (auto &item : re)
				item.ProbValue /= totProbVal;
			if (pdfTotVal)
				(*pdfTotVal) = totProbVal;
			return re;
		}

		Real IsPortside(XYXtd pos, Degree dir, array<Real, 2> toPos, Real epsilon)
		{
			Position A = pos.Pos;
			Position B = A.MoveTo(1000, dir);
			Position P{toPos[0], toPos[1]};

			Real dist = P.CrossTrackDistanceTo(A, B);
			if (dist > epsilon)
				return 1;
			else if (dist < epsilon)
				return -1;
			else
				return 0;
		}
	}

	// geospatial particle for computing emd
	template <typename Value = Real>
	struct GeospatialParticle
	{
		using value_type = Value;
		using Coords = array<Value, 2>;
		using Self = GeospatialParticle;

		// constructor from weight and coord array
		GeospatialParticle(Value weight, const Coords &xs) : weight_(weight), xs_(xs){};
		GeospatialParticle(Value weight, Value x, Value y) : weight_(weight), xs_({x, y}){};

		Value weight() const { return weight_; };

		const Value &operator[](ptrdiff_t i) const { return xs_[i]; };
		Value &operator[](ptrdiff_t i) { return xs_[i]; };

		static ptrdiff_t dimension() { return 2; };

		static Value plain_distance(const Self &p0, const Self &p1)
		{
			Position A{p0[0], p0[1]}, B{p1[0], p1[1]};
			Value dist = A.DistanceTo(B);
			return dist * dist;
		};

		static std::string name() { return "GeospatialParticle: [lon, lat]"; };
		static std::string distance_name() { return "HaversineDistance"; };

	protected:
		// store particle info
		Value weight_;
		Coords xs_;
	};

	using GeoParticle = GeospatialParticle<>;
	template <typename Value = Real>
	using GeoEvent = emd::EuclideanParticleEvent<GeospatialParticle<Value>>;
	template <typename Value = Real>
	using GeoDistance = emd::EuclideanParticleDistance<GeospatialParticle<Value>>;
	template <
		template <typename> class Event = GeoEvent,
		template <typename> class Distance = GeoDistance>
	using GeoEMD = emd::EMD<Real, Event, Distance>;

	Real WassersteinDistance(
		XYXtd a, Degree aDir,
		XYXtd b, Degree bDir,
		_WassersteinOption options)
	{
		using namespace emd;
		using namespace _hidden;

		// Alias of Wasserstein library (EMD: Earth-Mover Distance)
		using EMDParticle = emd::EuclideanParticle2D<>;
		using EMD = EMDFloat64<EuclideanEvent2D, EuclideanDistance2D>;

		// using GeoEMD = EMDReal<GeoEvent, GeoDistance>;

		/*using FelineEMD = EMD<Real,
			EuclideanParticleEvent<GeospatialParticle>, EuclideanParticleDistance<GeospatialParticle>
		>;*/
		int k = (int)floor(options.domainSize / options.domainUnit);
		int N = (2 * k + 1) * (2 * k + 1);

		auto SamplesA = GetBVNSamples(a, aDir, options.domainUnit, options.domainSize);
		auto SamplesB = GetBVNSamples(b, bDir, options.domainUnit, options.domainSize);

		vector<GeoParticle> ParticlesA;
		vector<GeoParticle> ParticlesB;

		for (int i = 0; i < N; i++)
		{
			GeoParticle tmpParticleA(SamplesA[i].ProbValue, SamplesA[i].Pos);
			GeoParticle tmpParticleB(SamplesB[i].ProbValue, SamplesB[i].Pos);

			ParticlesA.push_back(tmpParticleA);
			ParticlesB.push_back(tmpParticleB);
		}

		// build PDF as emd::Event form
		EuclideanParticleEvent<GeospatialParticle<>> eventA(ParticlesA);
		EuclideanParticleEvent<GeospatialParticle<>> eventB(ParticlesB);

		// EMD computing obj (Wasserstein library)
		GeoEMD<> EmdComputer = GeoEMD<>();
		auto reCheck = EmdComputer.compute(eventA, eventB);
		if (reCheck == emd::EMDStatus::Success)
			return EmdComputer.emd();
		else
			return numeric_limits<Real>::min();
	}

	Real JSDivergenceDistance(
		XYXtd a, Degree aDir,
		XYXtd b, Degree bDir,
		_JSDivergenceOption options)
	{
		using namespace _hidden;
		using namespace HashColon::Statistics;

		shared_ptr<Real> aPdfTot = make_shared<Real>(0);
		shared_ptr<Real> bPdfTot = make_shared<Real>(0);
		auto SamplesA = GetBVNSamples(a, aDir, options.domainUnit, options.domainSize, aPdfTot);
		auto SamplesB = GetBVNSamples(b, bDir, options.domainUnit, options.domainSize, bPdfTot);
		int k = (int)floor(options.domainSize / options.domainUnit);
		int N = (2 * k + 1) * (2 * k + 1);

		// JS divergence works in very small distance.
		// Therefore, no matter the distance type is, we will use cartesian distance
		// Base location for the cartesian distance is the mid-point of a and b
		Position distbase{(a.Pos.longitude + b.Pos.longitude) / 2, (a.Pos.latitude + b.Pos.latitude) / 2};

		// Compute sigma in each direction
		Real sigmaAP = a.Xtd.xtdPortside / options.domainSize;
		Real sigmaAS = a.Xtd.xtdStarboard / options.domainSize;
		Real sigmaAH = (sigmaAP + sigmaAS) / 2.0;
		Real sigmaBP = b.Xtd.xtdPortside / options.domainSize;
		Real sigmaBS = b.Xtd.xtdStarboard / options.domainSize;
		Real sigmaBH = (sigmaBP + sigmaBS) / 2.0;

		// Define directional points in H/S direciton
		const Real D = 10000; // 10km
		const Position AH = a.Pos.MoveTo(D, aDir);
		const Position AS = a.Pos.MoveTo(D, aDir + 90);
		const Position BH = b.Pos.MoveTo(D, bDir);
		const Position BS = b.Pos.MoveTo(D, bDir + 90);

		// Define BVN for z reference
		BVN zPDF;

		// sum( [PDF(A) * dA] * {log(PDF(A)) - log(PDF(M))} )
		Real KL_AM = 0;
		for (const auto &sample_ : SamplesA)
		{
			// define sample values
			const Position sample{sample_.Pos[0], sample_.Pos[1]};

			// compute distance in AH/AS/BH/BS to a & b from the sample
			Real zAH = sample.OnTrackDistanceTo(a.Pos, AH);
			Real zAS = sample.OnTrackDistanceTo(a.Pos, AS);
			Real zBH = sample.OnTrackDistanceTo(b.Pos, BH);
			Real zBS = sample.OnTrackDistanceTo(b.Pos, BS);

			// compute z-values
			zAH /= sigmaAH;
			zAS = zAS < options.errorEpsilon   ? zAS / sigmaAP
				  : zAS > options.errorEpsilon ? zAS / sigmaAS
											   : 0;
			zBH /= sigmaBH;
			zBS = zBS < options.errorEpsilon   ? zBS / sigmaBP
				  : zBS > options.errorEpsilon ? zBS / sigmaBS
											   : 0;

			// comput Pdf witth area
			Vector2R z;
			z << zBH, zBS;
			const Real PdfA_dArea = sample_.ProbValue;
			const Real PdfB_dArea = zPDF.PDF(z) / (*bPdfTot);

			// {log(PDF(A)) - log(PDF(M))}
			// = log(PDF(A) / PDF(M))
			// = log( PDF(A) * dAreaA / PDF(M) * dAreaA )
			// = log( PDF(A) * dAreaA ) - log( PDF(M) * dAreaA )
			// = log(PdfA_dArea) - log( .5 * (PDF(A) + PDF(B)) * dAreaA )
			// = log(PdfA_dArea) - log( .5 * PDF(A) * dAreaA + .5 * PDF(B)) * dAreaA )
			// = log(PdfA_dArea) - log( .5 * PdfA_dArea + .5 * PDF(B) * dAreaB * dAreaA / dAreaB )
			// = log(PdfA_dArea) - log( .5 * PdfA_dArea + .5 * PdfB_dArea * dAreaA / dAreaB )
			// = log(PdfA_dArea) - log( .5 * PdfA_dArea + .5 * PdfB_dArea * dAreaA / dAreaB )
			// = log(PdfA_dArea) - log( .5 * (PdfA_dA + PdfB_dArea * dAreaA / dAreaB ) )
			// dAreaA / dAreaB = sigmaAH * [sigmaAP|sigmaAS|.5*(sigmaAP+sigmaAS)) / sigmaBH * [sigmaBP|sigmaBS|.5*(sigmaBP+sigmaBS))

			// compute dAreaA / dAreaB
			Real dAreaA =
				zAS < options.errorEpsilon	 ? sigmaAH * sigmaAP
				: zAS > options.errorEpsilon ? sigmaAH * sigmaAS
											 : sigmaAH * sigmaAH;
			Real dAreaB =
				zBS < options.errorEpsilon	 ? sigmaBH * sigmaBP
				: zBS > options.errorEpsilon ? sigmaBH * sigmaBS
											 : sigmaBH * sigmaBH;

			// cummulate KL divergence AM
			KL_AM += PdfA_dArea * (log(PdfA_dArea) - log((PdfA_dArea + PdfB_dArea * dAreaA / dAreaB) / 2));
		}
		KL_AM = KL_AM > 0 ? KL_AM : 0;

		// sum( [PDF(B) * dB] * {log(PDF(B)) - log(PDF(M)) )
		Real KL_BM = 0;
		for (const auto &sample_ : SamplesB)
		{
			// define sample values
			const Position sample{sample_.Pos[0], sample_.Pos[1]};

			// compute distance in AH/AS/BH/BS to a & b from the sample
			Real zAH = sample.OnTrackDistanceTo(a.Pos, AH);
			Real zAS = sample.OnTrackDistanceTo(a.Pos, AS);
			Real zBH = sample.OnTrackDistanceTo(b.Pos, BH);
			Real zBS = sample.OnTrackDistanceTo(b.Pos, BS);

			// compute z-values
			zAH /= sigmaAH;
			zAS = zAS < options.errorEpsilon   ? zAS / sigmaAP
				  : zAS > options.errorEpsilon ? zAS / sigmaAS
											   : 0;
			zBH /= sigmaBH;
			zBS = zBS < options.errorEpsilon   ? zBS / sigmaBP
				  : zBS > options.errorEpsilon ? zBS / sigmaBS
											   : 0;

			// comput Pdf witth area
			Vector2R z;
			z << zAH, zAS;
			const Real PdfA_dArea = zPDF.PDF(z) / (*aPdfTot);
			const Real PdfB_dArea = sample_.ProbValue;

			// compute dAreaA / dAreaB
			Real dAreaA =
				zAS < options.errorEpsilon	 ? sigmaAH * sigmaAP
				: zAS > options.errorEpsilon ? sigmaAH * sigmaAS
											 : sigmaAH * sigmaAH;
			Real dAreaB =
				zBS < options.errorEpsilon	 ? sigmaBH * sigmaBP
				: zBS > options.errorEpsilon ? sigmaBH * sigmaBS
											 : sigmaBH * sigmaBH;

			// cummulate KL divergence AM
			KL_BM += PdfB_dArea * (log(PdfB_dArea) - log((PdfB_dArea + PdfA_dArea * dAreaB / dAreaA) / 2));
		}
		KL_BM = KL_BM > 0 ? KL_BM : 0;

		// return 0.5 * (KL_AM + KL_BM);
		return 0.5 * (KL_AM + KL_BM);
	}

	Real PFDistance(
		XYXtd a, Degree aDir,
		XYXtd b, Degree bDir,
		_PFDistanceOption options)
	{
		// Prevent PFDistance to be nan
		if (a.Pos == b.Pos)
			return 0;
		if (options.XtdSigmaRatio == 0)
			return 0;

		Degree a_pf_ab = a.Pos.AngleTo(b.Pos) - aDir;
		Real a_sigma = sqrt(
						   pow(0.5 * (a.Xtd.xtdPortside + a.Xtd.xtdStarboard) * cos(a_pf_ab), 2) +
						   pow(((a_pf_ab < 0) ? a.Xtd.xtdStarboard : a.Xtd.xtdPortside) * sin(a_pf_ab), 2)) *
					   options.XtdSigmaRatio;

		Degree b_pf_ba = b.Pos.AngleTo(a.Pos) - bDir;
		Real b_sigma = sqrt(
						   pow(0.5 * (b.Xtd.xtdPortside + b.Xtd.xtdStarboard) * cos(b_pf_ba), 2) +
						   pow(((b_pf_ba < 0) ? b.Xtd.xtdStarboard : b.Xtd.xtdPortside) * sin(b_pf_ba), 2)) *
					   options.XtdSigmaRatio;

		if (a_sigma * b_sigma == 0)
			return 0;
		return a.Pos.DistanceTo(b.Pos) * (a_sigma + b_sigma) / (2 * a_sigma * b_sigma);
	}
}

// XTD distance metric btwn trajectories
namespace HashColon::Feline::XtdTrajectoryClustering
{
	vector<XYXtdList> UniformSampling(
		vector<XYXtdList> &trajlist, size_t sampleNumber)
	{
		vector<XYXtdList> re;
		re.resize(trajlist.size());
#pragma omp parallel for
		for (size_t i = 0; i < trajlist.size(); i++)
		{
			re[i] = trajlist[i].GetUniformLengthSampled(sampleNumber);
		}
		return re;
	}

	Real XtdTrajectoryDistanceMeasureBase::Measure(
		const XYXtdList &a,
		const XYXtdList &b) const
	{
		Real re;
		if (_c.Enable_ReversedSequence)
		{
			re = min(
				Measure_core(a, b),
				Measure_core(a.GetReversed(), b));
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
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI("Feline.XtdTrajectoryDistanceMeasure");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--Enable_ReversedSequence", _cDefault.Enable_ReversedSequence,
						"Computes sequence-invariant measure. Computes min(D(A,B), D(A.rev, B))");
	}

	Real DtwXtd::Measure_core(
		const XYXtdList &a, const XYXtdList &b) const
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
					warp[i][j] = a[i].Pos.DistanceTo(b[j].Pos) + min({(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
																	  (j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
																	  ((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())});

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
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI("Feline.XtdTrajectoryDistanceMeasure.DtwXtd_JS");

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
		const XYXtdList &a, const XYXtdList &b) const
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
					aDir = (i == (a.size() - 1)) ? a[i - 1].Pos.AngleTo(a[i].Pos) : a[i].Pos.AngleTo(a[i + 1].Pos);
					bDir = (j == (b.size() - 1)) ? b[j - 1].Pos.AngleTo(b[j].Pos) : b[j].Pos.AngleTo(b[j + 1].Pos);

					warp[i][j] =
						JSDivergenceDistance(a[i], aDir, b[j], bDir,
											 {_c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon}) +
						min({(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
							 (j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
							 ((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())});

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
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI("Feline.XtdTrajectoryDistanceMeasure.DtwXtd_EMD");

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
		const XYXtdList &a, const XYXtdList &b) const
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
					aDir = (i == (a.size() - 1)) ? a[i - 1].Pos.AngleTo(a[i].Pos) : a[i].Pos.AngleTo(a[i + 1].Pos);
					bDir = (j == (b.size() - 1)) ? b[j - 1].Pos.AngleTo(b[j].Pos) : b[j].Pos.AngleTo(b[j + 1].Pos);

					warp[i][j] =
						WassersteinDistance(a[i], aDir, b[j], bDir,
											{_c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon}) +
						min({(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
							 (j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
							 ((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())});

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
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI("Feline.XtdTrajectoryDistanceMeasure.DtwXtd_Blended");

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
		const XYXtdList &a, const XYXtdList &b) const
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
					aDir = (i == (a.size() - 1)) ? a[i - 1].Pos.AngleTo(a[i].Pos) : a[i].Pos.AngleTo(a[i + 1].Pos);
					bDir = (j == (b.size() - 1)) ? b[j - 1].Pos.AngleTo(b[j].Pos) : b[j].Pos.AngleTo(b[j + 1].Pos);

					// debug
					Real D_Euclidean = a[i].Pos.DistanceTo(b[j].Pos);
					Real D_JS = JSDivergenceDistance(a[i], aDir, b[j], bDir,
													 {_c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon});
					Real D_PF = PFDistance(a[i], aDir, b[j], bDir, {_c.Pf_XtdSigmaRatio});
					Real D_WS = WassersteinDistance(a[i], aDir, b[j], bDir,
													{_c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon});

					warp[i][j] =
						(_c.Coeff_Euclidean > 0 ? _c.Coeff_Euclidean * a[i].Pos.DistanceTo(b[j].Pos) : 0) +
						(_c.Coeff_JS > 0 ? _c.Coeff_JS * JSDivergenceDistance(a[i], aDir, b[j], bDir,
																			  {_c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon})
										 : 0) +
						(_c.Coeff_PF > 0 ? _c.Coeff_PF * PFDistance(a[i], aDir, b[j], bDir, {_c.Pf_XtdSigmaRatio}) : 0) +
						(_c.Coeff_WS > 0 ? _c.Coeff_WS * WassersteinDistance(a[i], aDir, b[j], bDir,
																			 {_c.MonteCarloDomainUnit, _c.MonteCarloDomainSize, _c.MonteCarloErrorEpsilon})
										 : 0) +
						+min({(i >= 1 ? warp[i - 1][j] : numeric_limits<Real>::max()),
							  (j >= 1 ? warp[i][j - 1] : numeric_limits<Real>::max()),
							  ((i >= 1 && j >= 1) ? warp[i - 1][j - 1] : numeric_limits<Real>::max())});

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
