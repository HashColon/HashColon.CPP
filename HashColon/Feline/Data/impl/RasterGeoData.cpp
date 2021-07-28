#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif

#include <memory>
#include <mutex>
#include <vector>
#include <array>
#include <cassert>
#include <cmath>
#include <string>

#include <Eigen/Eigen>
// to use std::vector<Eigen::Matrix>
#include <Eigen/StdVector>

#include <gdal/gdal_priv.h>
#include <gdal/gdalwarper.h>
#include <gdal/cpl_conv.h>

#include <HashColon/Core/Real.hpp>
#include <HashColon/Core/ext/CLI11/CLI11_extended.hpp>
#include <HashColon/Helper/StringHelper.hpp>

#include <HashColon/Feline/Data/GdalSupport.hpp>

#include <iostream>

using namespace std;
using namespace HashColon;
using namespace HashColon::Helper;

// hidden functions for RasterGeoData
namespace _local
{
	/* 
	 * GDAL trasformation: 
	 * https://gdal.org/api/gdaldataset_cpp.html
	 * GDAL transformation T btwn projection coordinates X = (x, y) and raster index I = (i, j) is as follows
	 * x = T[0] + i * T[1] + j * T[2]
	 * y = T[3] + i * T[4] + j * T[5]
	 */

	array<Real, 2> Convert_RasterIdx2Position(
		array<Real, 2> RasterIdx,
		array<double, 6> gdalTransform)
	{
		/*
		 * x = T[0] + i * T[1] + j * T[2]
		 * y = T[3] + i * T[4] + j * T[5]
		 */
		array<Real, 2> re{
			gdalTransform[0] + RasterIdx[0] * gdalTransform[1] + RasterIdx[1] * gdalTransform[2],
			gdalTransform[3] + RasterIdx[0] * gdalTransform[4] + RasterIdx[1] * gdalTransform[5]
		};
		return re;
	}

	array<Real, 2> Convert_RasterIdx2Position(
		array<int, 2> RasterIdx,
		array<double, 6> gdalTransform)
	{
		array<Real, 2> ridx{ (Real)(RasterIdx[0]), (Real)(RasterIdx[1]) };
		return Convert_RasterIdx2Position(ridx, gdalTransform);
	}

	array<Real, 2> Convert_RasterIdx2Position(
		array<size_t, 2> RasterIdx,
		array<double, 6> gdalTransform)
	{
		array<Real, 2> ridx{ (Real)(RasterIdx[0]), (Real)(RasterIdx[1]) };
		return Convert_RasterIdx2Position(ridx, gdalTransform);
	}

	array<Real, 2> Convert_Position2RasterIdx(
		array<Real, 2> position,
		array<double, 6> gdalTransform)
	{
		/* 
		 * i = ( T[5] * (x - T[0]) - T[2] * (y - T[3]) ) / ( T[1] * T[5] - T[2] * T[4])
		 * j = ( T[1] * (y - T[3]) - T[4] * (x - T[0]) ) / ( T[1] * T[5] - T[2] * T[4])
		 */
		Real det = gdalTransform[1] * gdalTransform[5] - gdalTransform[2] * gdalTransform[4];
		array<Real, 2> re{
			(gdalTransform[5] * (position[0] - gdalTransform[0]) - gdalTransform[2] * (position[1] - gdalTransform[3])) / det,
			(gdalTransform[1] * (position[1] - gdalTransform[3]) - gdalTransform[4] * (position[0] - gdalTransform[0])) / det
		};
		return re;
	}

	array<size_t, 2> RoundIdx_TopLeft(array<Real, 2> idx)
	{
		return { (size_t)(floor(idx[0])), (size_t)(floor(idx[1])) };
	}

	array<size_t, 2> RoundIdx_Closest(array<Real, 2> idx)
	{
		return { (size_t)(round(idx[0])), (size_t)(round(idx[1])) };
	}
	
	Real BilinearInterpolation(Real u, Real v, vector<Real> p)
	{
		using namespace Eigen;
		// check if size of given p is 2*2
		assert(p.size() == 4);
		Matrix<Real, 2, 2, RowMajor> mat = Map<Matrix<Real, 2, 2, RowMajor>>(p.data(), 2, 2);
		//Matrix<Real, 2, 2> mat = Map<Matrix<Real, 2, 2>>(p.data(), 2, 2);
		RowVector2R uvec; uvec(0) = 1 - u; uvec(1) = u;
		Vector2R vvec; vvec(0) = 1 - v; vvec(1) = v;
		return uvec * mat * vvec;
	}

	template <typename T> GDALDataType GetGDALDataType();
	template <>	GDALDataType GetGDALDataType<unsigned char>() { return GDALDataType::GDT_Byte; }
	template <>	GDALDataType GetGDALDataType<unsigned short>() { return GDALDataType::GDT_UInt16; }
	template <>	GDALDataType GetGDALDataType<short>() { return GDALDataType::GDT_Int16; }
	template <>	GDALDataType GetGDALDataType<unsigned int>() { return GDALDataType::GDT_UInt32; }
	template <>	GDALDataType GetGDALDataType<int>() { return GDALDataType::GDT_Int32; }
	template <>	GDALDataType GetGDALDataType<float>() { return GDALDataType::GDT_Float32; }
	template <>	GDALDataType GetGDALDataType<double>() { return GDALDataType::GDT_Float64; }

	using cacheT = vector<vector<vector<Real>>>;
	

	void ResizeCache(cacheT& cache, size_t rasterCnt, size_t xSize, size_t ySize)
	{
		cache.resize(rasterCnt,
			vector<vector<Real>>(xSize,
				vector<Real>(ySize)
				)
		);
	}

	void ResizeCacheRaster(vector<vector<Real>>& raster, size_t xSize, size_t ySize)
	{
		raster.resize(xSize, vector<Real>(ySize));
	}

	vector<vector<Real>> BlockCache(const cacheT& cache, size_t rasterIdx, size_t x, size_t y, size_t xSize, size_t ySize)
	{
		vector<vector<Real>> re(xSize, vector<Real>(ySize));
		for (size_t i = 0; i < xSize; i++)
		{
			for (size_t j = 0; j < ySize; j++)
				re[i][j] = cache[rasterIdx][x + i][y + j];
		}
		return re;
	}
	
}

// RasterGeoData
namespace HashColon::Feline::Data
{
	RasterGeoData::RasterGeoData(string srcFilePath) 
		: _data(nullptr), _cached(false), _cdata()
	{
		std::call_once(RasterGeoData::_onlyOnce,
			[]() {
				GDALAllRegister();
				//GDALRegisterDriver()
			});

		_data = (GDALDataset*)GDALOpenShared(srcFilePath.c_str(), GA_ReadOnly);
	};

	RasterGeoData::~RasterGeoData()
	{		
		//_cdata.clear();
		/*for (size_t i = 0; i < _cdata.size(); i++)
		{
			for (size_t j = 0; j < _cdata[i].size(); j++)
			{
				_cdata[i][j].clear();
			}
			_cdata[i].clear();
		}
		_cdata.clear();*/
		GDALClose(_data);
		_data = 0;		
	};

	GDALDataset* RasterGeoData::GetRawGDALDataset() { return _data; };
	GDALRasterBand* RasterGeoData::GetRawGDALRasterBand(size_t rasterIdx)
	{
		return (_data->GetRasterBand((int)(rasterIdx + 1)));
	}

	// get number of channels(=layers=rasters)
	size_t RasterGeoData::GetRasterCount() const
	{
		return (size_t)(_data->GetRasterCount());
	};

	array<size_t, 2> RasterGeoData::GetRasterSize() const
	{
		array<size_t, 2> re;
		re[0] = _data->GetRasterXSize();
		re[1] = _data->GetRasterYSize();
		return re;
	}

	array<size_t, 2> RasterGeoData::GetRasterSize(size_t rasterIdx) const
	{
		assert(rasterIdx >= 0);
		int idx = (int)(rasterIdx + 1);
		array<size_t, 2> re;
		re[0] = _data->GetRasterBand(idx)->GetXSize();
		re[1] = _data->GetRasterBand(idx)->GetYSize();
		return re;
	}

	Real RasterGeoData::ValueAt(size_t rasterIdx,
		array<Real, 2> pos, ValueOption opt) const
	{
		switch (opt)
		{
		case vo_Closest:			
			return ValueAt(rasterIdx, _local::RoundIdx_Closest(pos)); break;
		case vo_Interpolate:
			array<Real, 2> idx_raw = IndexAtPosition(pos);
			array<size_t, 2> idx = _local::RoundIdx_TopLeft(idx_raw);
			Real u = idx_raw[0] - (Real)idx[0];
			Real v = idx_raw[1] - (Real)idx[1];

			if (_cached)
			{				
				return _local::BilinearInterpolation(u, v, _ValueBlockFromCacheAt(rasterIdx, idx[0], idx[1], 2, 2));
			}
			else
			{
				vector<Real> valblk;
				GDALDataType type = _data->GetRasterBand((int)(rasterIdx+1))->GetRasterDataType();

				switch (type)
				{
					case GDALDataType::GDT_Byte:	
					{
						vector<unsigned char> tmpblk = _ValueBlockFromRawAt<unsigned char>(rasterIdx, idx[0], idx[1], 2, 2);
						transform(tmpblk.begin(), tmpblk.end(), valblk.begin(), [](unsigned char x) { return (Real)x; });
					} break;
					case GDALDataType::GDT_UInt16:
					{
						vector<unsigned short> tmpblk = _ValueBlockFromRawAt<unsigned short>(rasterIdx, idx[0], idx[1], 2, 2);
						transform(tmpblk.begin(), tmpblk.end(), valblk.begin(), [](unsigned short x) { return (Real)x; });
					} break;					
					case GDALDataType::GDT_Int16:
					{
						vector<short> tmpblk = _ValueBlockFromRawAt<short>(rasterIdx, idx[0], idx[1], 2, 2);
						transform(tmpblk.begin(), tmpblk.end(), valblk.begin(), [](short x) { return (Real)x; });
					} break;					
					case GDALDataType::GDT_UInt32:
					{
						vector<unsigned int> tmpblk = _ValueBlockFromRawAt<unsigned int>(rasterIdx, idx[0], idx[1], 2, 2);
						transform(tmpblk.begin(), tmpblk.end(), valblk.begin(), [](unsigned int x) { return (Real)x; });
					} break;					
					case GDALDataType::GDT_Int32:
					{
						vector<int> tmpblk = _ValueBlockFromRawAt<int>(rasterIdx, idx[0], idx[1], 2, 2);
						transform(tmpblk.begin(), tmpblk.end(), valblk.begin(), [](int x) { return (Real)x; });
					} break;					
					case GDALDataType::GDT_Float32:
					{
						vector<float> tmpblk = _ValueBlockFromRawAt<float>(rasterIdx, idx[0], idx[1], 2, 2);
						transform(tmpblk.begin(), tmpblk.end(), valblk.begin(), [](float x) { return (Real)x; });
					} break;					
					case GDALDataType::GDT_Float64:
					{
						vector<double> tmpblk = _ValueBlockFromRawAt<double>(rasterIdx, idx[0], idx[1], 2, 2);
						transform(tmpblk.begin(), tmpblk.end(), valblk.begin(), [](double x) { return (Real)x; });
					} break;
					default:
						throw Exception("Unhandled GDAL data type.");
						break;
				}

				return Rawval2Val(rasterIdx, _local::BilinearInterpolation(u, v, valblk));
			}
			break;
		}

		throw Exception("Invalid value option for RasterGeoData.ValueAt()");
	}
	
	Real RasterGeoData::ValueAt(size_t rasterIdx, array<size_t, 2> idx) const
	{
		if (_cached)
		{
			return _ValueBlockFromCacheAt(rasterIdx, idx[0], idx[1])[0];
		}
		else
		{		
			Real val;
			GDALDataType type = _data->GetRasterBand((int)(rasterIdx+1))->GetRasterDataType();

			switch (type)
			{
			case GDALDataType::GDT_Byte:
				val = (Real)_ValueBlockFromRawAt<unsigned char>(rasterIdx, idx[0], idx[1])[0];
				break;
			case GDALDataType::GDT_UInt16:
				val = (Real)_ValueBlockFromRawAt<unsigned short>(rasterIdx, idx[0], idx[1])[0];
				break;
			case GDALDataType::GDT_Int16:
				val = (Real)_ValueBlockFromRawAt<short>(rasterIdx, idx[0], idx[1])[0];
				break;
			case GDALDataType::GDT_UInt32:
				val = (Real)_ValueBlockFromRawAt<unsigned int>(rasterIdx, idx[0], idx[1])[0];
				break;
			case GDALDataType::GDT_Int32:
				val = (Real)_ValueBlockFromRawAt<int>(rasterIdx, idx[0], idx[1])[0];
				break;
			case GDALDataType::GDT_Float32:
				val = (Real)_ValueBlockFromRawAt<float>(rasterIdx, idx[0], idx[1])[0];
				break;
			case GDALDataType::GDT_Float64:
				val = (Real)_ValueBlockFromRawAt<double>(rasterIdx, idx[0], idx[1])[0];
				break;
			default:
				throw Exception("Unhandled GDAL data type.");
				break;
			}

			return Rawval2Val(rasterIdx, val);
		}		
	}
	
	template<typename T>
	T RasterGeoData::RawValueAt(size_t rasterIdx, array<size_t, 2> idx) const
	{
		return _ValueBlockFromRawAt<T>(rasterIdx, idx[0], idx[1])[0];
	}

	vector<Real> RasterGeoData::_ValueBlockFromCacheAt(
		size_t rasterIdx, size_t xIdx, size_t yIdx, size_t xBlkSize, size_t yBlkSize) const
	{				
		return FlatBlockCache(rasterIdx, xIdx, yIdx, xBlkSize, yBlkSize);
	}

	template <typename T>
	vector<T> RasterGeoData::_ValueBlockFromRawAt(
		size_t rasterIdx, size_t xIdx, size_t yIdx, size_t xBlkSize, size_t yBlkSize) const
	{		
		assert(_data->GetRasterBand((int)(rasterIdx + 1))->GetXSize() > (int)(xIdx + xBlkSize));
		assert(_data->GetRasterBand((int)(rasterIdx + 1))->GetYSize() > (int)(yIdx + yBlkSize));
		GDALDataType bandtype = _local::GetGDALDataType<T>();
		int nbytes = sizeof(T);
		T* buffer = (T*)CPLMalloc(nbytes * xBlkSize * yBlkSize);

		//fetch raw data using RasterBand.RasterIO 
		CPLErr e = _data->GetRasterBand((int)(rasterIdx + 1))
			->RasterIO(
				GF_Read, 
				(int)xIdx, (int)yIdx, 
				(int)xBlkSize, (int)yBlkSize, buffer, 
				(int)xBlkSize, (int)yBlkSize, 
				bandtype, 0, 0);

		if (!(e == 0))
		{
			throw Exception(
				"RasterGeoData: Unable to read scanline in given geospatial file"
			);
		}

		vector<T> re(buffer, buffer + (xBlkSize * yBlkSize));
		CPLFree(buffer);

		return re;
	}

	Real RasterGeoData::Rawval2Val(size_t rasterIdx, Real v) const
	{
		Real scale = (Real)(_data->GetRasterBand((int)(rasterIdx+1))->GetScale());
		Real offset = (Real)(_data->GetRasterBand((int)(rasterIdx+1))->GetOffset());
		return scale * v + offset;
	}

	vector<Real> RasterGeoData::FlatBlockCache(
		size_t rasterIdx, size_t x, size_t y, size_t xSize, size_t ySize) const
	{
		vector<Real> re; re.resize(xSize * ySize);
		
		for (size_t i = 0; i < xSize; i++)
		{
			for (size_t j = 0; j < ySize; j++)
				re[i * ySize + j] = _cdata[rasterIdx][x + i][y + j];
		}
		return re;
	}
		
	bool RasterGeoData::IsCached() { return _cached; }
	void RasterGeoData::ClearCache() { _cdata.clear(); }

	template <typename T>
	void RasterGeoData::BuildCache(int rasterIdx)
	{
		int xsize = _data->GetRasterBand(rasterIdx+1)->GetXSize();
		int ysize = _data->GetRasterBand(rasterIdx+1)->GetYSize();
		//int tbyte = sizeof(T);
		Real scale = (Real)(_data->GetRasterBand(rasterIdx+1)->GetScale());
		Real offset = (Real)(_data->GetRasterBand(rasterIdx+1)->GetOffset());

		// resize _cdata[rasterIdx]
		_local::ResizeCacheRaster(_cdata[rasterIdx], xsize, ysize);		

		//T* buffer = (T*)CPLMalloc(tbyte * xsize);			
		T* buffer = new T[xsize];

		for (int row = 0; row < ysize; row++)
		{
			CPLErr e = _data->GetRasterBand(rasterIdx+1)->RasterIO(
				GF_Read, 0, row, xsize, 1, buffer, xsize, 1, 
				_data->GetRasterBand(rasterIdx+1)->GetRasterDataType(), 0, 0);
			if (!(e == 0)) {
				throw Exception(
					"RasterGeoData: Unable to read scanline in given geospatial file"
				);
			}

			for (int col = 0; col < xsize; col++)
			{
				_cdata[rasterIdx][col][row] = scale * (Real)(buffer[col]) + offset;
			}
		}
		//CPLFree(buffer);
		delete[] buffer;
	}

	void RasterGeoData::BuildCache()
	{
		assert(_data);
		int cnt = _data->GetRasterCount();
		_cdata.resize(cnt);
		
		for (int i = 0; i < cnt; i++)
		{
			GDALDataType type = _data->GetRasterBand(i+1)->GetRasterDataType();
			
			switch (type)
			{
			case GDALDataType::GDT_Byte:
				BuildCache<unsigned char>(i); 
				break;
			case GDALDataType::GDT_UInt16:
				BuildCache<unsigned short>(i); 
				break;
			case GDALDataType::GDT_Int16:
				BuildCache<short>(i); 
				break;
			case GDALDataType::GDT_UInt32:
				BuildCache<unsigned int>(i); 
				break;
			case GDALDataType::GDT_Int32:			
				BuildCache<int>(i); 
				break;
			case GDALDataType::GDT_Float32:
				BuildCache<float>(i); 
				break;
			case GDALDataType::GDT_Float64:
				BuildCache<double>(i);
				break;
			default:
				throw Exception("Unhandled GDAL data type.");
				break;
			}
		}	
		_cached = true;
	}

	array<Real, 2> RasterGeoData::PositionAtIndex(array<size_t, 2> idx) const
	{
		array<double, 6> t; _data->GetGeoTransform(t.data());
		return _local::Convert_RasterIdx2Position(idx, t);		
	}

	array<Real, 2> RasterGeoData::PositionAtIndex(array<Real, 2> idx) const
	{
		array<double, 6> t; _data->GetGeoTransform(t.data());
		return _local::Convert_RasterIdx2Position(idx, t);
	}
	array<Real, 2> RasterGeoData::IndexAtPosition(array<Real, 2> pos) const
	{
		array<double, 6> t; _data->GetGeoTransform(t.data());
		return _local::Convert_Position2RasterIdx(pos, t);
	}
	array<size_t, 2> RasterGeoData::IndexAtPosition(array<Real, 2> pos, IndexOption opt) const
	{
		array<double, 6> t; 

		assert(_data != nullptr);

		_data->GetGeoTransform(t.data());		
		array<Real,2> tmp = _local::Convert_Position2RasterIdx(pos, t);
		array<size_t, 2> re;
		switch (opt)
		{
		case idxo_Closest:
			re = _local::RoundIdx_Closest(tmp); break;
		case idxo_Topleft:
			re = _local::RoundIdx_TopLeft(tmp); break;
		default:
			throw Exception("Invalid index option");
		}

		auto size = GetRasterSize();

		if (re[0] >= size[0] || re[1] >= size[0]) 
			throw Exception("Given position exceeds raster boundary limit.");
		return re;		
	}

}

// SingletonRasterGeoData
namespace HashColon::Feline::Data
{
	namespace _srgd_helper
	{
		struct SingletonRasterLifeTimeManager
		{			
			unordered_map<string, shared_ptr<RasterGeoData>>* _list;

			SingletonRasterLifeTimeManager(
				unordered_map<string, shared_ptr<RasterGeoData>>* list) : _list(list) {};

			~SingletonRasterLifeTimeManager() 
			{					
				_list->clear();				
				ClearGDALRasterData(); 
			};
		};

	}
	

	SingletonRasterGeoData::SingletonRasterGeoData(const SingletonRasterGeoData& rs)
	{
		_instance = rs._instance;
	}

	SingletonRasterGeoData& SingletonRasterGeoData::operator=(const SingletonRasterGeoData& rs)
	{
		if (this != &rs)
		{
			_instance = rs._instance;
		}
		return *this;
	}

	shared_ptr<RasterGeoData> SingletonRasterGeoData::GetData_core(
		unordered_map<string, shared_ptr<RasterGeoData>>* dataset,
		const string dataname)
	{
		// check if the data with the given name exists 
		// else throw
		if ((*dataset).find(dataname) != (*dataset).end())
			return (*dataset)[dataname];
		else
			throw Exception("No raster data found with the given name <" + dataname + ">");
	}

	SingletonRasterGeoData& SingletonRasterGeoData::GetInstance(size_t id)
	{
		call_once(SingletonRasterGeoData::_onlyOnce,
			[](size_t idx)
			{
				SingletonRasterGeoData::_instance.reset(new SingletonRasterGeoData(idx));
			}, id);
		return *SingletonRasterGeoData::_instance;
	}

	void SingletonRasterGeoData::Initialize(const string configFilePath)
	{
		using namespace _srgd_helper;

		// Instantiate singleton
		GetInstance();

		// finalize is destructed when the program is terminated.
		// the destructor of SingletonRasterLifeTimeManager calls ClearGDALRasterData()
		static SingletonRasterLifeTimeManager finalize(
			&(GetInstance()._geodata));

		// use CLI11 to get configs
		// ** SingletonCLI is not used here. just in case.
		// don't forget to set formatter!!!! 
		CLI::App clihome("");
		clihome.config_formatter(std::make_shared<CLI::ConfigJson>());

		clihome.add_subcommand("Feline"); clihome.get_subcommand("Feline")->add_subcommand("Data");
		CLI::App* cli = clihome.get_subcommand("Feline")->get_subcommand("Data");
		cli->add_option("--RasterDataList", _c.namelist,
			"List of names of geodata. Number of items should match with --RasterFileList, --cacheOnLoad");
		cli->add_option("--RasterFileList", _c.filelist,
			"List of geodata files in raster format.  Number of items should match with --RasterDataList, --cacheOnLoad");

		cli->add_option_function<vector<string>>("--cacheOnLoad", [](const vector<string>& input) -> void
			{
				_c.cacheonload.clear(); _c.cacheonload.resize(input.size());
				for (size_t i = 0; i < input.size(); i++)
				{
					string tmp = ToLowerCopy(TrimCopy(input[i]));
					if (tmp == "true" || tmp == "yes" || tmp == "on" || tmp == "t")
					{
						_c.cacheonload[i] = true;
					}
					else if (tmp == "false" || tmp == "no" || tmp == "off" || tmp == "f")
					{
						_c.cacheonload[i] = false;
					}
					else
						throw Exception("Invalid boolean value");
				}
			}, "Set whether the geodata should be cached on load. List of boolean.  Number of items should match with --RasterDataList, --RasterFileList");
		
		clihome.set_config("--config", "", "Read configuration json file", true)->check(CLI::ExistingFile);
		clihome.allow_config_extras(true);
		clihome.callback(Initialize_callback);
		clihome.parse("--config " + configFilePath);		
	}

	void SingletonRasterGeoData::Initialize_callback()
	{
		using namespace _srgd_helper;

		// assert that the size of the arrays are same
		assert(_c.namelist.size() == _c.filelist.size());
		assert(_c.namelist.size() == _c.cacheonload.size());

		for (size_t i = 0; i < _c.namelist.size(); i++)
		{
			GetInstance()._geodata.insert({
				_c.namelist[i],
				make_shared<RasterGeoData>(_c.filelist[i]) });
			if (_c.cacheonload[i])
			{
				GetInstance()._geodata[_c.namelist[i]]->BuildCache();
			}
		}
	}

	shared_ptr<RasterGeoData> SingletonRasterGeoData::GetData(const std::string dataname)
	{
		return GetData_core(&_geodata, dataname);
	}

}

//function instantiations
namespace HashColon::Feline::Data
{	
	template <> unsigned char RasterGeoData::RawValueAt<unsigned char>(size_t rasterIdx, array<size_t,2> idx) const;
	template <> unsigned short RasterGeoData::RawValueAt<unsigned short>(size_t rasterIdx, array<size_t, 2> idx) const;
	template <> short RasterGeoData::RawValueAt<short>(size_t rasterIdx, array<size_t, 2> idx) const;
	template <> unsigned int RasterGeoData::RawValueAt<unsigned int>(size_t rasterIdx, array<size_t, 2> idx) const;
	template <> int RasterGeoData::RawValueAt<int>(size_t rasterIdx, array<size_t, 2> idx) const;
	template <> float RasterGeoData::RawValueAt<float>(size_t rasterIdx, array<size_t, 2> idx) const;
	template <> double RasterGeoData::RawValueAt<double>(size_t rasterIdx, array<size_t, 2> idx) const;
}

// clean up function
namespace HashColon::Feline::Data
{
	void ClearGDALRasterData()
	{
		GDALDestroyDriverManager();
	}
}