#pragma once
#include <string>
#include <map>
#include <pqxx/pqxx>


#include "Sos.h"

#include <iostream>
#include <fstream>

namespace MobileNoise
{
	template <typename dbType>
	class NoiseClassifier
	{
	public:
		// 1. make view of coords and ids of values according to time (or other criterion)
		// 2. get min and max coords values from view
		// 3. create partbounding boxes, exclude empty
		// 4. for each partbounding box do - this can be multithraded
		//	1. read geographies from map
		//	2. classify and save each value
		// 5. clean memory after work

		bool determineAffectedArea(const std::string & rDbConnStr, const std::string & rTableName, const std::string & rAffectedViewName, std::vector<std::string> & rCriteria);

		//bool runClassification(rMapDbReadingFunction, rDataDbReadingFunction, rClassifyingFunction, const std::map<double, double> & rBoundingBoxes, const std::string & rBboxSrid, const std::string & rMapDbSrid, const std::string & rDataDbSrid);
		bool clean(const std::string & rDbConnStr, std::string & rViewName);

		bool runSqlComm(pqxx::connection * C, const std::string & rFilepath)
		{
			try
			{
				if (!C->is_open())
				{
					 std::cout << "runClassification: Database not opened" << std::endl;
					 return false;
				}

				std::ifstream sqlf;
				sqlf.open (rFilepath);
				if (!sqlf.is_open())
				{
					 std::cout << "runClassification: File "<< rFilepath <<" not opened" << std::endl;
					 return false;
				}

				std::stringstream sql;
				sql << sqlf.rdbuf();    
				sqlf.close();

				/* Create a non-transactional object. */
				pqxx::nontransaction N(*C);
      
				/* Execute SQL query */
				pqxx::result R( N.exec( sql ));
      
				//return R.begin()[0].as<bool>();

				return true;
			}
			catch (std::exception & e)
			{
				 std::cout << "runClassification ERROR: " << e.what() << std::endl;
				 return false;
			}
		}

		bool runSqlComm(const std::string & rFilepath, const std::string & rDbConnStr)
		{
			try
			{

				std::ifstream sqlf;
				sqlf.open (rFilepath);
				if (!sqlf.is_open())
				{
					 std::cout << "runClassification: File "<< rFilepath <<" not opened" << std::endl;
					 return false;
				}

				std::stringstream sql;
				sql << sqlf.rdbuf();    
				sqlf.close();

				auto C = pqxx::connection(rDbConnStr);

				if (!C.is_open())
				{
					 std::cout << "runClassification: Database not opened" << std::endl;
					 return false;
				}

				/* Create a non-transactional object. */
				pqxx::nontransaction N(C);
      
				/* Execute SQL query */
				pqxx::result R( N.exec( sql ));
      
				//return R.begin()[0].as<bool>();
		
				C.disconnect ();
				return true;
			}
			catch (std::exception & e)
			{
				 std::cout << "runClassification ERROR: " << e.what() << std::endl;
				 return false;
			}
		}

		static time_t timestampConv(const std::string & rIsoTimestamp)
		{
			tm timeSt;
			size_t dotPos = rIsoTimestamp.find(".");
			std::string isoTimeStamp;
			if (dotPos != std::string::npos)
			{
				isoTimeStamp = rIsoTimestamp.substr(0, dotPos-1);
			}
			else
			{
				isoTimeStamp = rIsoTimestamp;
			}

			sscanf(isoTimeStamp.c_str(), "%d-%d-%d %d:%d:%d", &timeSt.tm_year, &timeSt.tm_mon, &timeSt.tm_mday, &timeSt.tm_hour, &timeSt.tm_min, &timeSt.tm_sec);

			timeSt.tm_year -= 1900;
			timeSt.tm_mon -= 1;
			timeSt.tm_isdst = 0;

			return std::mktime(&timeSt);

		}

		static std::string timestampConv(time_t rUnixTimestamp)
		{
			tm timeSt;
			timeSt = *std::gmtime(&rUnixTimestamp);
			//std::string isoTimeStamp;

			//sscanf(isoTimeStamp.c_str(), "%d-%d-%d %d:%d:%d", &timeSt.tm_year, &timeSt.tm_mon, &timeSt.tm_mday, &timeSt.tm_hour, &timeSt.tm_min, &timeSt.tm_sec);

			timeSt.tm_year += 1900;
			timeSt.tm_mon += 1;

			

			return std::to_string(timeSt.tm_year) + "-" + std::to_string(timeSt.tm_mon) + "-" + std::to_string(timeSt.tm_mday)  + " " 
				 + std::to_string(timeSt.tm_hour) + ":" + std::to_string(timeSt.tm_min) + ":" + std::to_string(timeSt.tm_sec);

		}

	};
}