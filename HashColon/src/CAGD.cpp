// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <array>
#include <cmath>
#include <memory>
#include <mutex>
#include <vector>
// dependant external libraries
#include <Eigen/Eigen>
// modified external libraries

// HashColon libraries
#include <HashColon/Exception.hpp>
#include <HashColon/Real.hpp>
// header file for this source file
#include <HashColon/CAGD.hpp>

using namespace std;
using namespace Eigen;
using namespace HashColon;

// SurfaceBasis
namespace HashColon::CAGD
{
	MatrixXR& SurfaceBasis::PreCompute(
		const vector<array<Real, 2>>& uv,
		bool noThrow)
	{
		preComputedValue = make_shared<MatrixXR>(Value(uv, noThrow));
		return *preComputedValue;
	}

	const MatrixXR& SurfaceBasis::PreComputedValue() const
	{
		if (!preComputedValue) throw Exception("Pre-computed value not computed yet");
		else return *preComputedValue;
	}
}

// Helper functions
namespace HashColon::CAGD::_helper
{
	VectorXR FlattenMatrixColwise(MatrixXR A)
	{	
		return VectorXR(Map<VectorXR>(A.data(), A.cols() * A.rows()));
	}

	VectorXR FlattenMatrixRowwise(Eigen::MatrixXR A)
	{		
		MatrixXR B = A.transpose();
		return VectorXR(Map<VectorXR>(B.data(), B.cols() * B.rows()));
	}

	array<vector<size_t>, 3> TriSurfaceIndexHelper::BIdx(size_t order)
	{
		using namespace std;
		// if no matching bidx info found, build one
		if (_bidxlist.find(order) == _bidxlist.end())
		{
			// boundary curve alias
			enum { cu = 0, cv = 1, cw = 2 };
			size_t& n = order;

			array<vector<size_t>, 3> bidx;
			// curve u: n - i - 1;			
			bidx[cu].resize(order);
			for (size_t i = 0; i < order; i++) { bidx[cu][i] = n - i - 1; }
			// curve v: ((n+1)*n - (n+1-i)*(n-i))/2
			//         = ((2n+1)*i - i*i)/2
			bidx[cv].resize(order);
			for (size_t i = 0; i < order; i++) { bidx[cv][i] = ((2 * n + 1) * i - i * i) / 2; }
			// curve w: ((n+1)*n - (i+1)*i)/2 - 1
			bidx[cw].resize(order);
			for (size_t i = 0; i < order; i++) { bidx[cw][i] = (n * (n + 1) - i * (i + 1)) / 2 - 1; }

			_bidxlist.insert({ order, bidx });
		}
		return _bidxlist[order];
	}

	vector<vector<size_t>>  TriSurfaceIndexHelper::VIdx(size_t order)
	{
		using namespace std;
		// if no matching idx info found, build one
		if (_uidxlist.find(order) == _uidxlist.end())
		{
			vector<vector<size_t>> uidx(order);
			size_t& n = order;

			for (size_t r = 0; r < n; r++)
			{
				uidx[r].resize(n - r);
				for (size_t i = 0; i < n - r; i++)
				{
					uidx[r][i] = ((2 * n + 1) * i - i * i) / 2 + r;
				}
			}

			_uidxlist.insert({ order, uidx });
		}
		return _uidxlist[order];
	}

	vector<vector<size_t>>  TriSurfaceIndexHelper::WIdx(size_t order)
	{
		using namespace std;
		// if no matching idx info found, build one
		if (_vidxlist.find(order) == _vidxlist.end())
		{
			vector<vector<size_t>> vidx(order);
			size_t& n = order;

			for (size_t r = 0; r < n; r++)
			{
				vidx[r].resize(n - r);
				for (size_t i = 0; i < n - r; i++)
				{
					vidx[r][i] = (n * (n + 1) - (i + r) * (i + r + 1)) / 2 - r - 1;
				}
			}

			_vidxlist.insert({ order, vidx });
		}
		return _vidxlist[order];
	}

	vector<vector<size_t>>  TriSurfaceIndexHelper::UIdx(size_t order)
	{
		using namespace std;
		// if no matching idx info found, build one
		if (_widxlist.find(order) == _widxlist.end())
		{
			vector<vector<size_t>> widx(order);
			size_t& n = order;

			for (size_t r = 0; r < n; r++)
			{
				widx[r].resize(n - r);
				for (size_t i = 0; i < n - r; i++)
				{
					widx[r][i] = (n * (n + 1) - (n - 1 - r) * (n - r)) / 2 - i - 1;
				}
			}

			_widxlist.insert({ order, widx });
		}
		return _widxlist[order];
	}

	array<vector<size_t>, 4> SurfaceIndexHelper::BIdx(array<size_t, 2> uvorders)
	{
		// if no matching bidx info found, build one
		if (_bidxlist.find(uvorders) == _bidxlist.end())
		{
			size_t& uOrder = uvorders[0];
			size_t& vOrder = uvorders[1];
			enum { u1 = 0, v1 = 1, u2 = 2, v2 = 3 };
			array<vector<size_t>, 4> bIdx;

			// build boundary indices 
			// u1 : { i * vOrder }
			bIdx[u1].resize(uOrder);
			for (size_t i = 0; i < uOrder; i++) bIdx[u1][i] = i * vOrder;
			// v1 : { (uOrder - 1) * vOrder + i }
			bIdx[v1].resize(vOrder);
			for (size_t i = 0; i < vOrder; i++) bIdx[v1][i] = (uOrder - 1) * vOrder + i;
			// u2 : { (uOrder - i) * vOrder - 1 }
			bIdx[u2].resize(uOrder);
			for (size_t i = 0; i < uOrder; i++) bIdx[u2][i] = (uOrder - i) * vOrder - 1;
			// v2 : { vOrder - i }
			bIdx[v2].resize(vOrder);
			for (size_t i = 0; i < vOrder; i++) bIdx[v2][i] = vOrder - i - 1;

			_bidxlist.insert({ uvorders, bIdx });
		}
		return _bidxlist[uvorders];
	}

	vector<vector<size_t>> SurfaceIndexHelper::UIdx(array<size_t, 2> uvorders)
	{
		// if no matching bidx info found, build one
		if (_uidxlist.find(uvorders) == _uidxlist.end())
		{
			size_t& uOrder = uvorders[0];
			size_t& vOrder = uvorders[1];

			vector<vector<size_t>> uidx;
			uidx.resize(vOrder);
			for (size_t i = 0; i < vOrder; i++)
			{
				uidx[i].resize(uOrder);
				for (size_t j = 0; j < uOrder; j++)
				{
					uidx[i][j] = vOrder * j + i;
				}
			}
			_uidxlist.insert({ uvorders, uidx });
		}
		return _uidxlist[uvorders];
	}

	vector<vector<size_t>> SurfaceIndexHelper::VIdx(array<size_t, 2> uvorders)
	{
		// if no matching bidx info found, build one
		if (_vidxlist.find(uvorders) == _vidxlist.end())
		{
			size_t& uOrder = uvorders[0];
			size_t& vOrder = uvorders[1];

			vector<vector<size_t>> vidx;
			vidx.resize(uOrder);
			for (size_t i = 0; i < uOrder; i++)
			{
				vidx[i].resize(vOrder);
				for (size_t j = 0; j < vOrder; j++)
				{
					vidx[i][j] = vOrder * i + j;
				}
			}
			_vidxlist.insert({ uvorders, vidx });
		}
		return _vidxlist[uvorders];
	}
}