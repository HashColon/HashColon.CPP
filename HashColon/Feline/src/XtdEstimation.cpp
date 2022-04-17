// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <cmath>
#include <queue>
#include <array>
// modified external libraries
#include <HashColon/CLI11.hpp>
#include <HashColon/CLI11_JsonSupport.hpp>
// HashColon libraries
#include <HashColon/Real.hpp>
#include <HashColon/SingletonCLI.hpp>
#include <HashColon/Feline/GeoData.hpp>
#include <HashColon/Feline/GeoValues.hpp>
// header file for this source file
#include <HashColon/Feline/XtdEstimation.hpp>

using namespace std;
using namespace HashColon;
using namespace HashColon::Feline;

namespace HashColon::Feline
{
	CATZOC::CATZOC_CLASS CATZOC::GetValue(Position p)
	{
		return CATZOC_B;
	}

	NavAreaType::NavArea_CLASS NavAreaType::GetValue(Position p)
	{
		Real d =
			SingletonRasterGeoData::GetInstance()
				.GetData("Bathymetry")
				->ValueAt(0, array<Real, 2>{p.dat[0], p.dat[1]});

		if (d < -200)
		{
			return NavArea_OpenSea;
		}
		else if (d < -50)
		{
			return NavArea_Coastal;
		}
		else if (d < 0)
		{
			return NavArea_HarborNConfinedWaters;
		}
		else
			return NavArea_HarborNConfinedWaters;
		// throw std::Exception("Given point is over-ground!");
	}
}

namespace HashColon::Feline
{
	XTD XTDEstimation::_Get_d_zoc(Position pos) const
	{
		return {
			_c.CatzocDistanceTable[CATZOC::GetValue(pos)][NavAreaType::GetValue(pos)],
			_c.CatzocDistanceTable[CATZOC::GetValue(pos)][NavAreaType::GetValue(pos)]};
	}

	XTD XTDEstimation::_Get_d_b(Real beam) const
	{
		return {beam, beam};
	}

	XTD XTDEstimation::_Get_d_pos() const
	{
		return {_c.PositionPrecision, _c.PositionPrecision};
	}

	XTD XTDEstimation::_Get_d_nsa(Position pos) const
	{
		return {
			_c.NavigationalSafetyAllowanceTable[NavAreaType::GetValue(pos)][0],
			_c.NavigationalSafetyAllowanceTable[NavAreaType::GetValue(pos)][1]};
	}

	XTD XTDEstimation::_Get_d_vsa(Real loa, Degree turnAngle) const
	{
		Real v = fabs(loa * sin((Radian)turnAngle));
		return {v, v};
	}

	namespace d_coast_helper
	{
		struct itemtype
		{
			std::array<size_t, 2> idx;
			XTD xtd;
		};
		struct XtdCompare
		{
			int operator()(itemtype a, itemtype b)
			{
				return max(a.xtd.xtdPortside, a.xtd.xtdStarboard) > max(b.xtd.xtdPortside, b.xtd.xtdStarboard);
			}
		};

		XTD getxtd_prim(Position A, Position B, array<Real, 2> P_arr)
		{
			Position P{P_arr[0], P_arr[1]};
			if (A.dat[0] == B.dat[0] && A.dat[1] == B.dat[1])
			{
				return {A.DistanceTo(P), A.DistanceTo(P)};
			}

			Real xtd_val = GeoDistance::CrossTrackDistance(P, A, B);
			Real otd_ratio = GeoDistance::OnTrackDistance(P, A, B) / A.DistanceTo(B); // on-track distance from A to P_stem in ratio of distance btwn AB

			if (otd_ratio < 0)
				return xtd_val > 0 ? XTD{A.DistanceTo(P), 0.0} : XTD{0.0, A.DistanceTo(P)};
			else if (otd_ratio > 1)
				return xtd_val > 0 ? XTD{B.DistanceTo(P), 0.0} : XTD{0.0, B.DistanceTo(P)};
			else
				return xtd_val > 0 ? XTD{xtd_val, 0} : XTD{0, -xtd_val};
		}

		array<array<int, 2>, 4> searchDir = {{{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};
	}

	XTD XTDEstimation::_Get_d_coast(
		Position pos, Position legEnd, XTD searchLimit, Real beam, Real draught) const
	{
		using namespace d_coast_helper;

		XTD re{0, 0};

		// priority_queue
		// {idx, XTD}

		priority_queue<itemtype, vector<itemtype>, XtdCompare> queue;

		// Bathymetry data
		RasterGeoData::Ptr gdata = SingletonRasterGeoData::GetInstance(_threadIdx).GetData("Bathymetry");

		// initialize visit checker as falase
		auto mapsize = gdata->GetRasterSize();
		vector<vector<bool>> vc;
		vc.resize(mapsize[0]);
		for (size_t i = 0; i < vc.size(); i++)
			vc[i].resize(mapsize[1], false);

		// set max XTD limit( given limit + margin)
		XTD maxlimit{searchLimit[0] + _c.DistanceMargin_D_coast, searchLimit[1] + _c.DistanceMargin_D_coast};
		XTD minlimit{beam + _c.DistanceMargin_D_coast, beam + _c.DistanceMargin_D_coast};

		// convert pos/legEnd as std::array<Real,2>
		// array<Real, 2> A{ pos[0], pos[1] };
		// array<Real, 2> B{ legEnd[0], legEnd[2] };
		Position &A = pos;
		Position &B = legEnd;

		// Get nearest pixel point from the waypoint and legEnd, then push it to the queue
		auto initIdx1 = gdata->IndexAtPosition({pos[0], pos[1]}, RasterGeoData::idxo_Closest);
		auto initPos1 = gdata->PositionAtIndex(initIdx1);
		auto initIdx2 = gdata->IndexAtPosition({legEnd[0], legEnd[1]}, RasterGeoData::idxo_Closest);
		auto initPos2 = gdata->PositionAtIndex(initIdx2);
		queue.push({initIdx1, getxtd_prim(A, B, initPos1)});
		queue.push({initIdx2, getxtd_prim(A, B, initPos2)});

		// Run BFS with distance priority to find closest grounding point
		do
		{
			// pop current
			itemtype current = queue.top();
			queue.pop();

			// check end condition
			// check if [ depth at current point > given depth ]
			if (gdata->ValueAt(0, current.idx) > -(_c.DraughtMargin_D_coast + draught))
			{
				// check if the both XTD distance is smaller than the margin.
				// this case is grounding path. throw exception.
				if (current.xtd[0] < minlimit[0] && current.xtd[1] < minlimit[0])
				{
					auto tmp = gdata->PositionAtIndex(current.idx);

					// ������ grounding check�� �ϰ� gounding route�� ���� ������ ���� ������,
					// ������ ���е��� �Ѱ�� �׳� �ּҰ� �����.
					// throw Exception("Path with grounding is given. invalid path");
					auto nsa = _Get_d_nsa({tmp[0], tmp[1]});
					return {minlimit.dat[0] + nsa.dat[0], minlimit.dat[1] + nsa.dat[1]};
				}

				// check if the current point is portside / starboard
				XTD tempxtd = getxtd_prim(A, B, gdata->PositionAtIndex(current.idx));

				// if the XTD of the current point side is vacant, assign it to return value
				re[0] = (re[0] == 0) ? tempxtd[0] : re[0];
				re[1] = (re[1] == 0) ? tempxtd[1] : re[0];

				// if both XTD of return value is filled, return.
				// else continue;
				if (re[0] != 0 && re[1] != 0)
					return re;
				else
					continue;
			}
			else
			{
				// push in new pixel points
				for (size_t i = 0; i < 4; i++)
				{
					// new pixel idx
					array<size_t, 2> newIdx{current.idx[0] + searchDir[i][0], current.idx[1] + searchDir[i][1]};

					// check if the pixel index exceeds the map size
					if (newIdx[0] >= mapsize[0] || newIdx[1] >= mapsize[1])
						continue;

					// check if visited
					if (vc[newIdx[0]][newIdx[1]])
						continue;
					else
						vc[newIdx[0]][newIdx[1]] = true;

					// get xtd distance
					// if pos != legEnd , then get distance btwn the new pixel point & the line segment(pos->legEnd)
					// else get distance btwn the new pixel point & pos
					// if the new pixel point is at portside, set XTD as (distance, 0)
					// else set XTD as (0, distance)
					XTD newXtd = getxtd_prim(A, B, gdata->PositionAtIndex(newIdx));

					// if new XTD exceeds limit, ignore
					if (newXtd[0] > maxlimit[0] || newXtd[1] > maxlimit[1])
						continue;

					// push {index, XTD} of new pixel
					queue.push({newIdx, newXtd});
				}
			}
		} while (!queue.empty());

		// fill the vacant values of return value with given XTD limit
		re[0] = (re[0] == 0) ? maxlimit[0] : re[0];
		re[1] = (re[1] == 0) ? maxlimit[1] : re[0];

		// return
		return re;
	}

	XTD XTDEstimation::EstimateXTD(
		Position pos, Position legEnd,
		Real loa, Real beam, Real draught, Degree turnAngle) const
	{
		XTD d_zoc = _Get_d_zoc(pos);
		XTD d_b = _Get_d_b(beam);
		XTD d_pos = _Get_d_pos();
		XTD d_nsa = _Get_d_nsa(pos);
		XTD d_vsa = _Get_d_vsa(loa, turnAngle);

		XTD xtd_computed{
			d_zoc.xtdPortside + d_b.xtdPortside + d_pos.xtdPortside + d_nsa.xtdPortside + d_vsa.xtdPortside,
			d_zoc.xtdStarboard + d_b.xtdStarboard + d_pos.xtdStarboard + d_nsa.xtdStarboard + d_vsa.xtdStarboard};

		XTD xtd_coast = _Get_d_coast(pos, legEnd, xtd_computed, beam, draught);

		return {
			min(xtd_computed.xtdPortside, xtd_coast.xtdPortside),
			min(xtd_computed.xtdStarboard, xtd_coast.xtdStarboard)};
	}

	XYXtdList XTDEstimation::EstimateXTD(
		XYList waypoints, Real loa, Real beam, Real draught) const
	{
		XYXtdList re;
		for (size_t i = 0; i < waypoints.size(); i++)
		{
			Degree turnangle =
				(i == 0 || i == waypoints.size() - 1) ? 0
													  : waypoints[i].AngleTo(waypoints[i + 1]) - waypoints[i - 1].AngleTo(waypoints[i]);
			XTD tmpXtd = (i == waypoints.size() - 1) ? EstimateXTD(waypoints[i], waypoints[i], loa, beam, draught, turnangle) : EstimateXTD(waypoints[i], waypoints[i + 1], loa, beam, draught, turnangle);

			XYXtd tmpXyxtd{waypoints[i], tmpXtd};
			re.push_back(tmpXyxtd);
		}
		return re;
	}

	void XTDEstimation::Initialize(const string configFilePath)
	{
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI("Feline.TrajectoryXtdEstimation");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		/*cli->add_option_function<vector<double>>("--CatzocDistanceTable",
			[&_cDefault](const vector<double>& input) -> void
			{
				size_t cnt = 0; _cDefault.CatzocDistanceTable.resize(5);
				for (size_t i = 0; i < 5; i++)
				{
					_cDefault.CatzocDistanceTable[i].resize(3);
					for (size_t j = 0; j < 3; j++)
					{
						_cDefault.CatzocDistanceTable[i][j] = input[cnt++];
					}
				}
			}
			, "Defined cross-track margin for each CATZOC type. Rows are for each CATZOC type(A1~D), columns are for navigational area type");*/
		cli->add_option_function<vector<double>>(
			"--CatzocDistanceTable",
			[](const vector<double> &input) -> void
			{
				size_t cnt = 0;
				_cDefault.CatzocDistanceTable.resize(5);
				for (size_t i = 0; i < 5; i++)
				{
					_cDefault.CatzocDistanceTable[i].resize(3);
					for (size_t j = 0; j < 3; j++)
					{
						_cDefault.CatzocDistanceTable[i][j] = input[cnt++];
					}
				}
			},
			"Defined cross-track margin for each CATZOC type. Rows are for each CATZOC type(A1~D), columns are for navigational area type");

		cli->add_option_function<vector<double>>(
			"--NavigationalSafetyAllowanceTable",
			[](const vector<double> &input) -> void
			{
				size_t cnt = 0;
				_cDefault.NavigationalSafetyAllowanceTable.resize(3);
				for (size_t i = 0; i < 3; i++)
				{
					_cDefault.NavigationalSafetyAllowanceTable[i].resize(2);
					for (size_t j = 0; j < 2; j++)
					{
						_cDefault.NavigationalSafetyAllowanceTable[i][j] = input[cnt++];
					}
				}
			},
			"Defined minimum cross-track limit distance determined by navigational area type. Harbor/confined area, coastal, & open sea.");

		cli->add_option("--PositionPrecision", _cDefault.PositionPrecision,
						"Position precision determined by position source. ex) GNSS, GPS etc.");

		cli->add_option("--DistanceMargin_D_coast", _cDefault.DistanceMargin_D_coast,
						"Distance margin while computing minimum cross-track distance to shore.");

		cli->add_option("--DraughtMargin_D_coast", _cDefault.DraughtMargin_D_coast,
						"Draught margin while computing minimum cross-track distance to shore.");
	}
}
