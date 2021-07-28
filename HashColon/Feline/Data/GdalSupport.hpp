#ifndef HASHCOLON_FELINE_DATA_GDALSUPPORT_HPP
#define HASHCOLON_FELINE_DATA_GDALSUPPORT_HPP

#include <memory>
#include <array>
#include <string>
#include <mutex>
#include <unordered_map>

#include <Eigen/Eigen>
// to use std::vector<Eigen::Matrix>
#include <Eigen/StdVector>

#include <gdal/gdal_priv.h>
#include <gdal/gdalwarper.h>

#include <HashColon/Core/Real.hpp>
#include <HashColon/Core/Exception.hpp>

#include <iostream>

namespace HashColon::Feline::Data
{	
	class RasterGeoData
	{
	public:
		using Ptr = std::shared_ptr<RasterGeoData>;
		enum ValueOption
		{
			// Interpolate values of nearby pixels 
			vo_Interpolate, 
			// Get the value of closest pixel
			vo_Closest
		};
		enum IndexOption
		{
			// Get the top left pixel of nearby pixels. Used for computing interpolatio value
			idxo_Topleft, 
			// Get the index of closest pixel
			idxo_Closest
		};
				
		RasterGeoData(std::string srcFilePath);

		~RasterGeoData();

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(RasterGeoData);
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	private:
		// for lazy initialization for GDAL (to run GDALAlllRegister)
		static inline std::once_flag _onlyOnce;

	protected:
		GDALDataset* _data;
		bool _cached = false;
		std::vector<std::vector<std::vector<HashColon::Real>>> _cdata;		
		
				
		std::vector<HashColon::Real> _ValueBlockFromCacheAt(
			size_t rasterIdx, size_t xIdx, size_t yIdx, 
			size_t xBlkSize = 1, size_t yBlkSize = 1) const;

		template<typename T = HashColon::Real>
		std::vector<T> _ValueBlockFromRawAt(size_t rasterIdx, size_t xIdx, size_t yIdx,
			size_t xBlkSize = 1, size_t yBlkSize = 1) const;

		inline HashColon::Real Rawval2Val(size_t rasterIdx, HashColon::Real v) const;

		inline std::vector<HashColon::Real> FlatBlockCache(
			size_t rasterIdx, size_t x, size_t y, size_t xSize, size_t ySize) const;
		
	public:		

		GDALDataset* GetRawGDALDataset();
		GDALRasterBand* GetRawGDALRasterBand(size_t rasterIdx=0);
		
		// get number of channels(=layers=rasters)
		size_t GetRasterCount() const;
		std::array<size_t, 2> GetRasterSize() const;
		std::array<size_t, 2> GetRasterSize(size_t rasterIdx) const;

		void BuildCache();
		template <typename T>
		void BuildCache(int rasterIdx);
		bool IsCached();
		void ClearCache();
		
		// get value from gdal raster object. Scale and Offset is applied to Value.
		HashColon::Real ValueAt(size_t rasterIdx,
			std::array<HashColon::Real, 2> position, 
			ValueOption opt = vo_Interpolate) const;
		// get value from gdal raster object. Scale and Offset is applied to Value.		
		HashColon::Real ValueAt(size_t rasterIdx, std::array<size_t,2> idx) const;
				
		/*
		 * Get raw value from gdal raster object. Use ValueAt() instead of this function to retrieve the intended value.
		 * Plz check GDALRasterBand.GetRasterDataType() and give appropriate T
		 */
		template<typename T>
		T RawValueAt(size_t rasterIdx, std::array<size_t, 2> idx) const;

		std::array<HashColon::Real, 2> PositionAtIndex(std::array<size_t, 2> idx) const;
		std::array<HashColon::Real, 2> PositionAtIndex(std::array<HashColon::Real, 2> idx) const;
		std::array<HashColon::Real, 2> IndexAtPosition(std::array<HashColon::Real, 2> pos) const;
		std::array<size_t, 2> IndexAtPosition(std::array<HashColon::Real, 2> pos, IndexOption opt) const;
	};

	void ClearGDALRasterData();
	
}

namespace HashColon::Feline::Data
{
	class SingletonRasterGeoData
	{
	public:
		struct _params {
			std::vector<std::string> namelist;
			std::vector<std::string> filelist;
			std::vector<bool> cacheonload;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(SingletonRasterGeoData);

	private:
		static inline _params _c;
		static inline  std::shared_ptr<SingletonRasterGeoData> _instance;
		static inline std::once_flag _onlyOnce;
		std::unordered_map<std::string, std::shared_ptr<RasterGeoData>> _geodata;

		SingletonRasterGeoData(size_t id) {};;
		SingletonRasterGeoData(const SingletonRasterGeoData& rs);

		SingletonRasterGeoData& operator=(const SingletonRasterGeoData& rs);
		std::shared_ptr<RasterGeoData> GetData_core(std::unordered_map<std::string, std::shared_ptr<RasterGeoData>>* dataset, const std::string dataname);

	public:
		~SingletonRasterGeoData() {  };
		static SingletonRasterGeoData& GetInstance(size_t id = 0);
		static void Initialize(const std::string configFilePath = "");
		static void Initialize_callback();
		std::shared_ptr<RasterGeoData> GetData(const std::string dataname);

	};
}

#endif