#include <exception>
#include <pqxx/pqxx>
#include <iostream>
#include <sstream>

#include "NoiseReaderWriter.h"

namespace MobileNoise
{
	template<>
	std::string NoiseReaderWriter<pqxx::connection>::createArgumentBatch(std::vector<std::string> & rArguments, const std::string & rBeforeFirstArg, const std::string & rBetweenArgs, const std::string & rAfterArgs, const std::string & rEmptyArgs)
	{
		std::stringstream output;
		output << rBeforeFirstArg;
		if ((rArguments.empty()) || ((rArguments.size() == 1) && (rArguments[0] == "*")))
		{
			output << rEmptyArgs;
		}
		else
		{
			size_t size = rArguments.size();
			for (size_t i = 0; i < size; i++)
			{
				output << " " << rArguments[i];
				if (i == size-1)
				{
					break;//output << " ";
				}
				else
				{
					output << rBetweenArgs;
				}
			}
		}
		output << rAfterArgs;
		return output.str();
	}

	template<>
	bool NoiseReaderWriter<pqxx::connection>::initialize()
	{
		return createConnection(mDbConnStr, &C);
	}

	template<>
	bool NoiseReaderWriter<pqxx::connection>::createConnection(const std::string & rDbConnStr, pqxx::connection * C)
	{
		 try
		 {
			 C = new pqxx::connection(rDbConnStr);
			 return true;
		 }
		 catch (std::exception & e)
		 {
			 std::cout << "NoseReaderWriter::createConnecton ERROR: " << e.what() << std::endl;
			 return false;
		 }
	}

	template<>
	bool NoiseReaderWriter<pqxx::connection>::closeConnection(pqxx::connection * C)
	{
		try
		{
			C->disconnect ();
			return true;
		}
		catch (std::exception & e)
		{
			 std::cout << "NoseReaderWriter::closeConnecton ERROR: " << e.what() << std::endl;
			 return false;
		}
	}

	template<>
	bool NoiseReaderWriter<pqxx::connection>::createView(std::vector<std::string> & rTableNames, const std::string & rViewName, std::vector<std::string> & rCriteria, std::vector<std::string> & rVisibleColumns)
	{
		return createView(&C, rTableNames, rViewName, rCriteria, rVisibleColumns);
	}

	template<>
	bool NoiseReaderWriter<pqxx::connection>::createView(pqxx::connection * C, std::vector<std::string> & rTableNames, const std::string & rViewName, std::vector<std::string> & rCriteria, std::vector<std::string> & rVisibleColumns)
	{

		try
		{
			if (!C->is_open()) 
			{
				std::cout << "NoiseReaderWriter<pqxx::connection>::createView: Can't open database for view " << rViewName << std::endl;
				return false;
			}
		

			std::stringstream viewStr;
			viewStr << "CREATE OR REPLACE VIEW " << rViewName << " AS SELECT ";

			viewStr << createArgumentBatch(rVisibleColumns, " ", ", ", " ", "*");

			viewStr << " FROM ";

			viewStr << createArgumentBatch(rTableNames, " ", ", ", " ", "*");

			if (!rCriteria.empty())
			{
				viewStr << createArgumentBatch(rCriteria, " WHERE ", " AND ", " ", "");
			}

			viewStr << ";";


			/* Create a transactional object. */
			pqxx::work W(*C);
      
			/* Execute SQL query */
			W.exec( viewStr );
			W.commit();
			std::cout << "View " << rViewName << " created successfully" << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << "NoiseReaderWriter<pqxx::connection>::createView: ERROR: " << e.what() << std::endl;
			return false;
		}
		return true;
	}

	template<>
	bool NoiseReaderWriter<pqxx::connection>::dropView(pqxx::connection * C, const std::string & rViewName)
	{
		try
		{
			if (!C->is_open()) 
			{
				std::cout << "NoiseReaderWriter<pqxx::connection>::dropView: Can't open database for view " << rViewName << std::endl;
				return false;
			}
		

			std::stringstream viewStr;
			viewStr << "DROP VIEW " << rViewName << ";";


			/* Create a transactional object. */
			pqxx::work W(*C);
      
			/* Execute SQL query */
			W.exec( viewStr );
			W.commit();
			std::cout << "View " << rViewName << " dropped successfully" << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << "NoiseReaderWriter<pqxx::connection>::dropView: ERROR: " << e.what() << std::endl;
			return false;
		}
		return true;
	}

	template<>
	bool NoiseReaderWriter<pqxx::connection>::getGivenDataBoundingBox(pqxx::connection * C, const std::string & rGeomColName,const std::string & rTableNames, const std::string & rWhereClause, MobileNoise::SOS::Envelope & rBoundBox)
	{
		try
		{
			if (!C->is_open()) 
			{
				std::cout << "NoiseReaderWriter<pqxx::connection>::getGivenDataBoundingBox: Can't open database for table " << rTableNames << std::endl;
				return false;
			}
		

			std::stringstream viewStr;
			viewStr << "select max(ST_X(" << rGeomColName << ")), max(ST_Y(" << rGeomColName << ")), min(ST_X(" << rGeomColName << ")), min(ST_Y(" << rGeomColName << "))";
			viewStr << " from " << rTableNames;
			
			if (!rWhereClause.empty())
			{
				viewStr<< " where " << rWhereClause;
			}
			viewStr << ";";

			/* Create a non-transactional object. */
			pqxx::nontransaction N(*C);
      
			/* Execute SQL query */
			pqxx::result R( N.exec( viewStr ));

			auto row = R.begin();
      
			rBoundBox.maxVals.longitude = row[0].as<double>();
			rBoundBox.maxVals.latitude = row[1].as<double>();
			rBoundBox.minVals.longitude = row[2].as<double>();
			rBoundBox.minVals.latitude = row[3].as<double>();


			std::cout << "Data is in " << rBoundBox.maxVals.longitude << ", " << rBoundBox.maxVals.latitude << ", " << rBoundBox.minVals.longitude << ", " << rBoundBox.minVals.latitude << " bounds" << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << "NoiseReaderWriter<pqxx::connection>::getGivenDataBoundingBox: ERROR: " << e.what() << std::endl;
			return false;
		}
		return true;
	}

	template <>
	static bool NoiseReaderWriter<pqxx::connection>::transform_point(pqxx::connection * C, MobileNoise::SOS::Point & rPoint, const std::string & rToSrid, const std::string & rFromSrid)
	{
		try
		{
			if (!C->is_open()) 
			{
				std::cout << "NoiseReaderWriter<pqxx::connection>::transform_point: Can't open database " << std::endl;
				return false;
			}

			std::string fromSrid;
			if (!rFromSrid.empty())
			{
				fromSrid = rFromSrid;
			}
			else
			{
				fromSrid= rPoint.getSrid();
			}
			
			std::string toSrid = rToSrid;

			if (toSrid == "degrees_loc")
			{
				toSrid = "4326";
			}

			if (fromSrid == "degrees_loc")
			{
				fromSrid = "4326";
			}

			// recompute according to srid
			if ((!fromSrid.empty()) && (!toSrid.empty()) && (fromSrid != toSrid) )  
			{
				std::stringstream sql;
				sql << "SELECT ST_AsGeoJSON(ST_Transform(ST_SetSRID(ST_Point(" << rPoint.longitude << ", " << rPoint.latitude << ")," << fromSrid << ")," << toSrid << "))";


				/* Create a non-transactional object. */
				pqxx::nontransaction N(*C);
      
				/* Execute SQL query */
				pqxx::result Ra( N.exec( sql ));

				auto row = Ra.begin();
      
				sql.str(std::string());
				sql << row[0].as<std::string>();
				
				Json::Value Json;
				sql >> Json;

				SOS::Point Pt;
				Pt.fromJson(Json);
				Pt.srid = toSrid;
			}

		}
		catch (const std::exception &e)
		{
			std::cout << "NoiseReaderWriter<pqxx::connection>::transform_point: ERROR: " << e.what() << std::endl;
			return false;
		}
		return true;
	}


	template <>
	bool NoiseReaderWriter<pqxx::connection>::getGeometryData_point(pqxx::connection * C, const std::string & rTableName, const MobileNoise::SOS::Envelope & rBoundBox, std::vector<MobileNoise::SOS::Point> & rGeometryData)
	{
		return false;
	}

	template <>
	bool NoiseReaderWriter<pqxx::connection>::setGeometryData_point(pqxx::connection * C, const std::string & rTableName, std::vector<MobileNoise::SOS::Point> & rGeometryData)
	{
		return false;
	}

	template <>
	bool NoiseReaderWriter<pqxx::connection>::getGeometryData_polyline(pqxx::connection * C, const std::string & rTableName, const MobileNoise::SOS::Envelope & rBoundBox, std::vector<MobileNoise::SOS::Point> & rGeometryData)
	{
		return false;
	}

	template <>
	bool NoiseReaderWriter<pqxx::connection>::setGeometryData_polyline(pqxx::connection * C, const std::string & rTableName, std::vector<MobileNoise::SOS::Point> & rGeometryData)
	{
		return false;
	}


} // namespace MobileNoise