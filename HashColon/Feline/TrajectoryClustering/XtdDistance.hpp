#ifndef HASHCOLON_FELINE_XTDDISTANCE_HPP
#define HASHCOLON_FELINE_XTDDISTANCE_HPP

#include <vector>
#include <HashColon/Core/Real.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>

namespace HashColon::Feline::XtdTrajectoryClustering
{
	/* JS-divergence distance */
	struct _JSDivergenceOption
	{
		HashColon::Real domainUnit;
		HashColon::Real domainSize;
		HashColon::Real errorEpsilon;
	};

	const _JSDivergenceOption _cDefault_JSDivergence = {
		1.0, 3.0, 1e-6
	};;

	HashColon::Real JSDivergenceDistance(
		HashColon::Feline::XYXtd a, HashColon::Degree aDir,
		HashColon::Feline::XYXtd b, HashColon::Degree bDir,
		_JSDivergenceOption options = _cDefault_JSDivergence);

	/* Wasserstein distance */
	struct _WassersteinOption
	{
		HashColon::Real domainUnit;
		HashColon::Real domainSize;
		HashColon::Real errorEpsilon;		
	};
		
	const _WassersteinOption _cDefault_Wasserstein = {
		1.0, 3.0, 1e-6
	};
	
	HashColon::Real WassersteinDistance(
		HashColon::Feline::XYXtd a, HashColon::Degree aDir,
		HashColon::Feline::XYXtd b, HashColon::Degree bDir,
		_WassersteinOption options = _cDefault_Wasserstein);

	/* PF distance */
	struct _PFDistanceOption
	{
		HashColon::Real XtdSigmaRatio;
	}; 

	const _PFDistanceOption _cDefault_PFDistance = { 3.0 };

	HashColon::Real PFDistance(
		HashColon::Feline::XYXtd a, HashColon::Degree aDir,
		HashColon::Feline::XYXtd b, HashColon::Degree bDir,
		_PFDistanceOption options = _cDefault_PFDistance);

}

#endif