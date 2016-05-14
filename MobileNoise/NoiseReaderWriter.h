#pragma once
#include <string>
#include <vector>

#include "NoiseData.h"

namespace MobileNoise
{
	template <typename dbType> class NoiseReaderWriter
	{
	public:
		bool initialize();
		bool close();
		bool getBoundingBox(MobileNoise::SOS::Envelope & rBoundBox);
		template <typename T>
		bool getGeometryData(const MobileNoise::SOS::Envelope & rBoundBox, std::vector<T> & rGeometryData);
		template <typename T>
		bool setGeometryData(std::vector<T> & rGeometryData);
		bool createView(std::vector<std::string> & rTableNames, const std::string & rViewName, std::vector<std::string> & rCriteria, std::vector<std::string> & rVisibleColumns);
		
		NoiseReaderWriter(const std::string & rDbConnStr)
			: mDbConnStr(rDbConnStr)
		{

		}

		dbType C;
		std::string mDbConnStr;
		                                                
	private:

		bool createConnection(const std::string & rDbConnStr, dbType * c);
		bool closeConnection(dbType * C);
		bool createView(dbType * C, std::vector<std::string> & rTableNames, const std::string & rViewName, std::vector<std::string> & rCriteria, std::vector<std::string> & rVisibleColumns);
		bool dropView(dbType * C, const std::string & rViewName);

		bool getGivenDataBoundingBox(dbType * C, const std::string & rGeomColName, const std::string & rTableNames, const std::string & rWhereClause, MobileNoise::SOS::Envelope & rBoundBox);

		bool getNoiseData_point(dbType * C, const std::string & rTableName, const MobileNoise::SOS::Envelope & rBoundBox, std::vector<NoisePoint> & rGeometryData);

		bool getGeometryData_point(dbType * C, const std::string & rTableName, const MobileNoise::SOS::Envelope & rBoundBox, std::vector<SOS::Point> & rGeometryData);
		bool setGeometryData_point(dbType * C, const std::string & rTableName, std::vector<SOS::Point> & rGeometryData);

		bool getGeometryData_polyline(dbType * C, const std::string & rTableName, const MobileNoise::SOS::Envelope & rBoundBox, std::vector<SOS::Point> & rGeometryData);
		bool setGeometryData_polyline(dbType * C, const std::string & rTableName, std::vector<SOS::Point> & rGeometryData);

		static std::string createArgumentBatch(std::vector<std::string> & rArguments, const std::string & rBeforeFirstArg, const std::string & rBetweenArgs, const std::string & rAfterArgs, const std::string & rEmptyArgs);
		static bool transform_point(dbType * C, MobileNoise::SOS::Point & rPoint, const std::string & rToSrid, const std::string & rFromSrid = "");
		static bool transform_polyline(dbType * C, MobileNoise::SOS::Point & rPoint, const std::string & rToSrid, const std::string & rFromSrid = "");
		

		//template <typename T> bool readValue(


	};
}