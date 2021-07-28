#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <HashColon/Feline/Feline_config.h>
#include <cassert>
#include <cmath>
#include <mutex>
#include <Eigen/Eigen>
#include <Wasserstein/Wasserstein.hh>
#include <HashColon/Feline/Types/ValueTypes.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>
#include <HashColon/Statistics/NomalDistribution.hpp>
#include <HashColon/Feline/TrajectoryClustering/XtdDistance.hpp>

using namespace std;
using namespace Eigen;
using namespace HashColon;
using namespace HashColon::Helper;
using namespace HashColon::Feline;

mutex _XtdDistance_cpp_mutex;

namespace HashColon::Feline::XtdTrajectoryClustering
{
	namespace _hidden
	{
		static unique_ptr<vector<vector<Real>>> zBvn;
		static int zk;

		struct SampleType
		{
			array<Real, 2> Pos;
			Real ProbValue;
		};

		vector<vector<Real>> PreComputeZNormals(int k)
		{	
			Statistics::BVN bvn;			
			vector<vector<Real>> re(2 * k + 1, vector<Real>(2 * k + 1));

			for (int i = -k; i <= k; i++)
				for (int j = -k; j <= k; j++)
				{
					Eigen::Vector2R tmp;
					tmp << (Real)i, (Real)j;
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
			Real stepSize,		// sample rate in unit of sigma
			Real domainSize)	// size of the domain box in unit of sigma			
		{
			assert(domainSize > 0);
			assert(stepSize > 0);

			// Get base vectors
			Position pos = point.Pos;
			Degree aS = direction + 90; // starboard direction
			Degree aH = direction;
			Real sigmaP = point._Xtd.xtdPortside / domainSize;		// set sigma in portside direction as 1/3 of xtd portside
			Real sigmaS = point._Xtd.xtdStarboard / domainSize;		// set sigma in starboard direction  as 1/3 of xtd starboard
			Real sigmaH = (sigmaP + sigmaS) / 2;				// set sigma in fwd/back direction as average of p/s sigma1
			int k = (int)floor(domainSize / stepSize);
			size_t N = (2 * k + 1) * (2 * k + 1);

			if (k != zk || !zBvn)
			{
				lock_guard<mutex> _lg(_XtdDistance_cpp_mutex);
				zk = k;
				zBvn = make_unique<vector<vector<Real>>>(PreComputeZNormals(k));				
			}

			vector<SampleType> re; re.reserve(N);

			for (int i = -k; i <= k; i++)
			{
				for (int j = -k; j <= k; j++)
				{
					Real x = i <= 0 ? sigmaP * stepSize * i : sigmaS * stepSize * i;
					Real y = sigmaH * stepSize * j;
					Position tmpPos = pos.MoveTo(aS, x).MoveTo(aH, y);
					//SampleType tmp{ {{tmpPos.dat[0] * _LonUnitDist, tmpPos.dat[1] * _LatUnitDist}}, (*zBvn)[i + k][j + k] };
					SampleType tmp;
					/*tmp.Pos[0] = tmpPos.dat[0] * _LonUnitDist;
					tmp.Pos[1] = tmpPos.dat[1] * _LatUnitDist;*/
					tmp.Pos[0] = tmpPos.dat[0];
					tmp.Pos[1] = tmpPos.dat[1];
					{
						lock_guard<mutex> _lg(_XtdDistance_cpp_mutex);
						tmp.ProbValue = (*zBvn)[i + k][j + k];
					}
					//cout << i << ", " << j << endl;
					re.push_back(tmp);
				}
			}
			return re;
		}

		Real IsPortside(XYXtd pos, Degree dir, array<Real, 2> toPos, Real epsilon)
		{
			Position A = pos._Pos;
			Position B = A.MoveTo(dir, 1000);
			Position P; P.dat = toPos;

			Real dist = P.CrossTrackDistanceTo(A, B);
			if (dist > epsilon) return 1;
			else if (dist < epsilon) return -1;
			else return 0;
		}
	}

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
		int k = (int)floor(options.domainSize / options.domainUnit);
		int N = (2 * k + 1) * (2 * k + 1);

		auto SamplesA = GetBVNSamples(a, aDir, options.domainUnit, options.domainSize);
		auto SamplesB = GetBVNSamples(b, bDir, options.domainUnit, options.domainSize);

		vector<EMDParticle> ParticlesA;
		vector<EMDParticle> ParticlesB;
		Real testA = 0, testB = 0;

		for (int i = 0; i < N; i++)
		{
			EMDParticle tmpParticleA(SamplesA[i].ProbValue, SamplesA[i].Pos[0] * _LonUnitDist, SamplesA[i].Pos[1] * _LatUnitDist);
			EMDParticle tmpParticleB(SamplesB[i].ProbValue, SamplesB[i].Pos[0] * _LonUnitDist, SamplesB[i].Pos[1] * _LatUnitDist);

			ParticlesA.push_back(tmpParticleA);
			ParticlesB.push_back(tmpParticleB);

			testA += SamplesA[i].ProbValue;
			testB += SamplesB[i].ProbValue;
		}

		// build PDF as emd::Event form	
		EuclideanParticleEvent eventA(ParticlesA);
		EuclideanParticleEvent eventB(ParticlesB);

		// EMD computing obj (Wasserstein library)
		EMD EmdComputer = EMD();
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
				
		auto SamplesA = GetBVNSamples(a, aDir, options.domainUnit, options.domainSize);
		auto SamplesB = GetBVNSamples(b, bDir, options.domainUnit, options.domainSize);
		int k = (int)floor(options.domainSize / options.domainUnit);
		int N = (2 * k + 1) * (2 * k + 1);

		// Build PDF
		Vector2R muA; muA << a._Pos.longitude * _LonUnitDist, a._Pos.latitude * _LatUnitDist;
		Vector2R muB; muB << b._Pos.longitude * _LonUnitDist, b._Pos.latitude * _LatUnitDist;
		Real sigmaAP = a._Xtd.xtdPortside / options.domainSize;
		Real sigmaAS = a._Xtd.xtdStarboard / options.domainSize;
		Real sigmaAH = (sigmaAP + sigmaAS) / 2.0;
		Real sigmaBP = b._Xtd.xtdPortside / options.domainSize;
		Real sigmaBS = b._Xtd.xtdStarboard / options.domainSize;
		Real sigmaBH = (sigmaBP + sigmaBS) / 2.0;
		Matrix2R SigmaAP; SigmaAP << sigmaAP * sigmaAP, 0, 0, sigmaAH * sigmaAH;
		Matrix2R SigmaAS; SigmaAS << sigmaAS * sigmaAS, 0, 0, sigmaAH * sigmaAH;
		Matrix2R SigmaBP; SigmaBP << sigmaBP * sigmaBP, 0, 0, sigmaBH * sigmaBH;
		Matrix2R SigmaBS; SigmaBS << sigmaBS * sigmaBS, 0, 0, sigmaBH * sigmaBH;
		BVN PdfAP(muA, SigmaAP);
		BVN PdfAS(muA, SigmaAS);
		BVN PdfBP(muB, SigmaBP);
		BVN PdfBS(muB, SigmaBS);

		// sum( [PDF(A) * dA] * {log(PDF(A)) - log(PDF(M)) )
		Real KL_AM = 0;		
		for (int i = 0; i < N; i++)
		{
			// [PDF(A) * dA]
			Position PosA; PosA.dat = SamplesA[i].Pos;
			Position PosB = b._Pos;
			Position PosBH = PosB.MoveTo(bDir, 1000);
			Position PosBS = PosB.MoveTo(bDir - 90, 1000);
			Real zBH = PosA.CrossTrackDistanceTo(PosB, PosBH);
			Real zBS = PosA.CrossTrackDistanceTo(PosB, PosBS);
			BVN zPDF;
			Real PdfA, PdfB ,PdfM;
			Real PdfA_dArea = SamplesA[i].ProbValue;
			PdfA = SamplesA[i].ProbValue;

			if (zBS < options.errorEpsilon)
			{
				Vector2R z; z << zBH / sigmaBH, zBS / sigmaBP;
				PdfB = zPDF.PDF(z);
			}
			else if (zBS > options.errorEpsilon)
			{
				Vector2R z; z << zBH / sigmaBH, zBS / sigmaBS;
				PdfB = zPDF.PDF(z);
			}
			else
			{
				Vector2R z; z << zBH / sigmaBH, 0;
				PdfB = zPDF.PDF(z);
			}
			PdfM = (PdfA + PdfB) / 2.0;
			
			Real logPdfA = log(PdfA);
			Real logPdfM = log(PdfM);
			KL_AM += PdfA_dArea * (logPdfA - logPdfM);
		}

		// sum( [PDF(B) * dB] * {log(PDF(B)) - log(PDF(M)) )
		Real KL_BM = 0;		
		for (int i = 0; i < N; i++)
		{
			// [PDF(A) * dA]
			Real PdfB_dArea = SamplesB[i].ProbValue;
			
			/* ver 2.0 */
			Position PosA = a._Pos;
			Position PosB; PosB.dat = SamplesB[i].Pos;			
			Position PosAH = PosA.MoveTo(aDir, 1000);
			Position PosAS = PosA.MoveTo(aDir - 90, 1000);
			Real zAH = PosB.CrossTrackDistanceTo(PosA, PosAH);
			Real zAS = PosB.CrossTrackDistanceTo(PosA, PosAS);
			BVN zPDF;
			Real PdfA, PdfB, PdfM;
			PdfB = SamplesB[i].ProbValue;
			if (zAS < options.errorEpsilon)
			{
				Vector2R z; z << zAH / sigmaAH, zAS / sigmaAP;
				PdfA = zPDF.PDF(z);
			}
			else if (zAS > options.errorEpsilon)
			{
				Vector2R z; z << zAH / sigmaAH, zAS / sigmaAS;
				PdfA = zPDF.PDF(z);
			}
			else
			{
				Vector2R z; z << zAH / sigmaAH, 0;
				PdfA = zPDF.PDF(z);
			}
			PdfM = (PdfA + PdfB) / 2.0;

			Real logPdfB = log(PdfB);
			Real logPdfM = log(PdfM);
			KL_BM += PdfB_dArea * (logPdfB - logPdfM);
		}

		//return 0.5 * (KL_AM + KL_BM);
		return 0.5 * (KL_AM + KL_BM);
	}

	Real PFDistance(
		XYXtd a, Degree aDir,
		XYXtd b, Degree bDir,
		_PFDistanceOption options)
	{	
		// Prevent PFDistance to be nan
		if (a._Pos == b._Pos) return 0;
		if (options.XtdSigmaRatio == 0) return 0;

		Degree a_pf_ab = a.AngleTo(b._Pos) - aDir;
		Real a_sigma = sqrt(
			pow(0.5 * (a._Xtd.xtdPortside + a._Xtd.xtdStarboard) * cos(a_pf_ab), 2) +
			pow(((a_pf_ab < 0) ? a._Xtd.xtdStarboard : a._Xtd.xtdPortside) * sin(a_pf_ab), 2)
		) * options.XtdSigmaRatio;	

		Degree b_pf_ba = b.AngleTo(a._Pos) - bDir;
		Real b_sigma = sqrt(
			pow(0.5 * (b._Xtd.xtdPortside + b._Xtd.xtdStarboard) * cos(b_pf_ba), 2) +
			pow(((b_pf_ba < 0) ? b._Xtd.xtdStarboard : b._Xtd.xtdPortside) * sin(b_pf_ba), 2)
		) * options.XtdSigmaRatio;		

		if (a_sigma * b_sigma == 0) return 0;
		return a.DistanceTo(b._Pos) * (a_sigma + b_sigma) / (2 * a_sigma * b_sigma);
	}


}