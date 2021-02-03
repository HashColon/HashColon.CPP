#include <HashColon/Feline/Feline_config.h>
#include <cassert>
#include <algorithm>
#include <HashColon/Helper/SimpleVector.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>

using namespace HashColon::Helper;

namespace HashColon::Feline::Types::Simple
{
	Real XYList::GetDistance(int sIndex, int eIndex) const
	{
		// check validity					
		if (eIndex == -1)
			eIndex = this->size() - 1;
		assert(sIndex >= 0 && sIndex < this->size());
		assert(eIndex >= 0 && eIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
			re += at(i).DistanceTo(at(i + 1));

		return re;
	}

	Real XYTList::GetDistance(int sIndex, int eIndex) const
	{
		// check validity					
		if (eIndex == -1)
			eIndex = this->size() - 1;
		assert(sIndex >= 0 && sIndex < this->size());
		assert(eIndex >= 0 && eIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
			re += at(i).DistanceTo(at(i + 1)._Pos);

		return re;
	}

	Duration XYTList::GetDuration(int sIndex, int eIndex) const
	{
		// check validity					
		if (eIndex == -1)
			eIndex = this->size() - 1;
		assert(sIndex >= 0 && sIndex < this->size());
		assert(eIndex >= 0 && eIndex < this->size());
		assert(sIndex <= eIndex);

		TimePoint sTP = at(sIndex)._TP;
		TimePoint eTP = at(eIndex)._TP;

		Duration re = eTP - sTP;
		return re;
	}

	XYList XYTList::ToXYList() const
	{
		XYList re;
		re.clear();
		for (XYT it : (*this))
		{
			re.push_back(it._Pos);
		}
		return re;
	}

	Real XYVVaTList::GetDistance(int sIndex, int eIndex) const
	{
		// check validity					
		if (eIndex == -1)
			eIndex = this->size() - 1;
		assert(sIndex >= 0 && sIndex < this->size());
		assert(eIndex >= 0 && eIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
		{
			re += at(i).DistanceTo(at(i + 1)._Pos);
		}
		return re;
	}

	Duration XYVVaTList::GetDuration(int sIndex, int eIndex) const
	{
		// check validity					
		if (eIndex == -1)
			eIndex = this->size() - 1;
		assert(sIndex >= 0 && sIndex < this->size());
		assert(eIndex >= 0 && eIndex < this->size());
		assert(sIndex <= eIndex);

		TimePoint sTP = at(sIndex)._TP;
		TimePoint eTP = at(eIndex)._TP;

		Duration re = eTP - sTP;
		return re;
	}

	XYList XYVVaTList::ToXYList() const
	{
		XYList re;
		re.clear();
		for (XYVVaT it : *(this))
		{
			re.push_back(it._Pos);
		}
		return re;
	}

	XYTList XYVVaTList::ToXYTList() const
	{
		XYTList re;
		re.clear();
		for (XYVVaT it : *(this))
		{
			XYT temp(it._Pos, it._TP);
			re.push_back(temp);
		}
		return re;
	}

	XYList XYList::GetNormalizedXYList(int sizeN) const
	{
		XYList re;
		re.clear();  re.resize(sizeN + 1);

		// Step 1. Parameterization of the route
		std::vector<Real> uVec = GetParameterized();
		Real stepu = GetDistance() / ((Real)sizeN);

		// Step 2. Point sample from paraemeter u = k * stepL;
		size_t ptcnt1 = 0;
		size_t ptcnt2 = 0;
		size_t ptcnt2_old = 0;
		Real u = 0.0;
		for (int i = 0; i < sizeN; u += stepu, i++)
		{
			// uVec[ptcnt] < u < uVec[ptcnt + 1]
			ptcnt2_old = ptcnt2;
			while (u >= uVec[ptcnt2] && ptcnt2 < uVec.size()-1)
			{
				ptcnt2++;
			}
			ptcnt1 = ptcnt2_old != ptcnt2 ? ptcnt2_old : ptcnt1;

			{
				using namespace HashColon::Helper::Vec2D;
				/*               uVec[ptcnt2] - u                               u - uVec[ptcnt1]
				pt[ptcnt1] * ----------------------------- + pt[ptcnt2] * -----------------------------
				uVec[ptcnt2] - uVec[ptcnt1]                   uVec[ptcnt2] - uVec[ptcnt1]
				*/
				re[i].dat = plus<Real>(
					multiply<Real>((uVec[ptcnt2] - u) / (uVec[ptcnt2] - uVec[ptcnt1]), this->at(ptcnt1).dat),
					multiply<Real>((u - uVec[ptcnt1]) / (uVec[ptcnt2] - uVec[ptcnt1]), this->at(ptcnt2).dat));
			}
		}

		re[sizeN] = this->back();

		return re;
	}

	std::vector<Real> XYList::GetParameterized() const
	{	
		std::vector<Real> uVec;
		uVec.clear(); uVec.resize(size());

		for (int i = 0; i < size(); i++)
			uVec[i] = GetDistance(0, i);

		return uVec;
	}

	XYList XYList::GetReversed() const
	{
		XYList re;
		//re.reserve(size());
		re.resize(size());
		std::reverse_copy(begin(), end(), re.begin());
		return re;
	}

	XYTList XYTList::GetReversed() const
	{
		XYTList re;
		re.reserve(size());
		std::reverse_copy(begin(), end(), re.begin());
		return re;
	}

	XYVVaTList XYVVaTList::GetReversed() const
	{
		XYVVaTList re;
		re.reserve(size());
		std::reverse_copy(begin(), end(), re.begin());
		return re;
	}

}
