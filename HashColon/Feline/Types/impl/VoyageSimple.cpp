#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <HashColon/Feline/Feline_config.h>
#include <cassert>
#include <algorithm>
#include <Eigen/Eigen>
#include <HashColon/Helper/SimpleVector.hpp>
#include <HashColon/Feline/Types/ValueTypes.hpp>
#include <HashColon/Feline/Types/VoyageSimple.hpp>

using namespace std;
using namespace HashColon::Helper;

// hidden
namespace HashColon::Feline::_hidden
{
	bool operator==(const _XYT& lhs, const _XYT& rhs)
	{
		return lhs._Pos == rhs._Pos && lhs._TP == rhs._TP;
	}
	bool operator!=(const _XYT& lhs, const _XYT& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator==(const _XYVVaT& lhs, const _XYVVaT& rhs)
	{
		return lhs._Pos == rhs._Pos && lhs._TP == rhs._TP && lhs._Vel == rhs._Vel;
	}
	bool operator!=(const _XYVVaT& lhs, const _XYVVaT& rhs)
	{
		return !(lhs == rhs);
	}

	bool operator==(const _XYXtd& lhs, const _XYXtd& rhs)
	{
		return lhs._Pos == rhs._Pos && lhs._Xtd == rhs._Xtd;
	}
	bool operator!=(const _XYXtd& lhs, const _XYXtd& rhs)
	{
		return !(lhs == rhs);
	}
}

// XYLists
namespace HashColon::Feline
{
	Real XYList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());		
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
			re += at(i).DistanceTo(at(i + 1));

		return re;
	}

	XYList XYList::GetNormalizedXYList(size_t sizeN) const
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
		for (size_t i = 0; i < sizeN; u += stepu, i++)
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

		for (size_t i = 0; i < size(); i++)
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
}

// XYTLists
namespace HashColon::Feline
{
	Real XYTList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
			re += at(i).DistanceTo(at(i + 1)._Pos);

		return re;
	}

	Duration XYTList::GetDuration(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
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
		for (const XYT& it : (*this))
		{
			re.push_back(it._Pos);
		}
		return re;
	}
	XYTList XYTList::GetReversed() const
	{
		XYTList re;
		re.reserve(size());
		std::reverse_copy(begin(), end(), re.begin());
		return re;
	}
}

// XYVVaTLists
namespace HashColon::Feline
{
	Real XYVVaTList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
		{
			re += at(i).DistanceTo(at(i + 1)._Pos);
		}
		return re;
	}

	Duration XYVVaTList::GetDuration(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
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
		for (const XYVVaT& it : *(this))
		{
			re.push_back(it._Pos);
		}
		return re;
	}

	XYTList XYVVaTList::ToXYTList() const
	{
		XYTList re;
		re.clear();
		for (const XYVVaT& it : *(this))
		{
			XYT temp(it._Pos, it._TP);
			re.push_back(temp);
		}
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

// XYXtdLists
namespace HashColon::Feline
{
	Real XYXtdList::GetDistance(size_t sIndex, size_t eIndex) const
	{
		// check validity					
		if (eIndex >= this->size())
			eIndex = this->size() - 1;
		assert(sIndex < this->size());
		assert(sIndex <= eIndex);

		Real re = 0;
		for (size_t i = sIndex; i < eIndex; i++)
			re += at(i).DistanceTo(at(i + 1)._Pos);

		return re;
	}

	XYList XYXtdList::ToXYList() const
	{
		XYList re;
		re.clear();
		for (const XYXtd& it : (*this))
		{
			re.push_back(it._Pos);
		}
		return re;
	}

	XYXtdList XYXtdList::GetNormalizedXYXtdList(size_t sizeN) const
	{
		XYXtdList re;
		re.clear();  re.resize(sizeN + 1);

		// Step 1. Parameterization of the route
		std::vector<Real> uVec = ToXYList().GetParameterized();
		Real stepu = GetDistance() / ((Real)sizeN);

		// Step 2. Point sample from paraemeter u = k * stepL;
		size_t ptcnt1 = 0;
		size_t ptcnt2 = 0;
		size_t ptcnt2_old = 0;
		Real u = 0.0;
		for (size_t i = 0; i < sizeN; u += stepu, i++)
		{
			// uVec[ptcnt] < u < uVec[ptcnt + 1]
			ptcnt2_old = ptcnt2;
			while (u >= uVec[ptcnt2] && ptcnt2 < uVec.size() - 1)
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
				assert(uVec.size() > ptcnt1);
				assert(uVec.size() > ptcnt2);
				assert(size() > ptcnt1);
				assert(size() > ptcnt2);

				array<Real, 2> tmppos = plus<Real>(
					multiply<Real>((uVec[ptcnt2] - u) / (uVec[ptcnt2] - uVec[ptcnt1]), this->at(ptcnt1)._Pos.dat),
					multiply<Real>((u - uVec[ptcnt1]) / (uVec[ptcnt2] - uVec[ptcnt1]), this->at(ptcnt2)._Pos.dat));
				Position temp_pos = Position(tmppos[0], tmppos[1]);
				XTD temp_xtd = this->at(ptcnt1)._Xtd;
				re[i] = XYXtd(temp_pos, temp_xtd);
			}
		}

		re[sizeN] = this->back();

		return re;
	}

	XYXtdList XYXtdList::GetReversed() const
	{
		XYXtdList re;
		//re.reserve(size());
		re.resize(size());
		std::reverse_copy(begin(), end(), re.begin());
		return re;
	}
}
