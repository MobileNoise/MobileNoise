#pragma once

#include "Sos.h"

#include <pqxx/pqxx>

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <curl.h>
#include <atomic>

#include "CachingStream.h"

#define USE_CHUNKED

namespace MobileNoise
{
	namespace SOS
	{
		static const std::string OBS_STR = "observations\"";
		//static const std::string OBS_FILENAME = "GetObservationsResponse.json";

		class SosReader
		{

		public:


			int pullAllData(const std::string & sqlConnectionString, const std::string & sosServiceUrl, const std::string & requestJsonFilePath, const std::string & obsIdent = "", const std::string & resultObsProp = "", const std::string & resultUom = "");

			struct WriteThis 
			{
				const char *readptr;
				long long sizeleft;
			};

			SosReader();

			template <typename Meas>
			int pullData(const std::string & sqlConnectionString, const std::string & sosServiceUrl, const std::string & requestJsonFilePath, const std::string & obsIdent = "", const std::string & resultObsProp = "", const std::string & resultUom = "")
			{
				std::ifstream sosInput(requestJsonFilePath);
				if (sosInput.is_open())
				{
					int err = 2;
					std::stringstream reqSosJson;
					char ch;
					while (sosInput.get(ch))
					{
						reqSosJson << ch;
					}

					isConnected = true;
					std::thread readThread(&MobileNoise::SOS::SosReader::getObsFromServer, this, sosServiceUrl, reqSosJson.str());
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					//err = getObsFromServer(sosServiceUrl, reqSosJson.str());
					err = readSosData<Meas>(sqlConnectionString, obsIdent, resultObsProp, resultUom);
					if (readThread.joinable())
					{

						readThread.join();
					}
					return err;

				}

				std::cout << "JSON SOS request file " << requestJsonFilePath << " not found." << std::endl;
				return 1;
			}

			template <typename Meas>
			int readSosData(const std::string & sqlConnStr, const std::string & obsIdent = "", const std::string & resultObsProp = "", const std::string & resultUom = "")
			{

				if (cs.hasData() || isConnected)
				{
				  int err = 2;
				  char c;

				  // searching for "observations"
				  while (cs.get(c))    
				  {
					  if ('\"' == c)
					  {
						  bool found = false;
						  size_t index = 0;
			  
						  while (cs.get(c))
						  {
							  if (c == OBS_STR[index])
							  {
								  index++;
								  continue;
							  }
							  else if (index == OBS_STR.size())
							  {
								  found = true;
								  break;
							  }
							  found = false;
							  break;
						  }

						  if (found)
						  {
							  err = 0;
							  break;
						  }
			 
					  }
				  }

				  if (err != 0)
				  {
					  cs.terminate();
					  std::cout << "Observations in datas not found." <<std::endl;
					  return err;
				  }

				  bool obsNotFound = true;

				  // searching for first "["
				  while (cs.get(c))    
				  {
					  if ('[' == c)
					  {
						  obsNotFound = false;
						  break;
					  }
				  }

				  if (obsNotFound)
				  {
					  cs.terminate();
					  std::cout << "Observations in datas not found." <<std::endl;
					  return err;
				  }

				  try{
					  pqxx::connection C(sqlConnStr);

					  bool run = createMeasTablesIfNotExist(&C);

					  std::vector<Json::Value> observations;
					  while (run)
					  {
						  std::stringstream obsSs = getOneObservationJson();

						  obsSs.seekg(0, std::ios::end);
						  std::streamoff size = obsSs.tellg();
						  if (0 == size)
						  {
							  break;
						  }
						  obsSs.seekg(0, std::ios::beg);
						  Json::Value obsJs;
						  obsSs >> obsJs;
						  //MobileNoise::SOS::OM_Measurement<double,MobileNoise::SOS::Point> meas;
						  //observations.push_back(obsJs);

						  saveObsIntoDB<Meas>(obsJs, &C, obsIdent, resultObsProp, resultUom);

						  //std::cout << obsJs << std::endl << "----------------------------------" << std::endl;
					  }
					  C.disconnect ();
				  }
				  catch(const std::exception &e)
				  {
					  std::cout << e.what() << std::endl;
					  return 3;
				  }
				  cs.terminate();
				  return 0;
			  }
			  std::cout << "No data available" <<std::endl;
			  return 1;

			}
		
		private:
			bool createMeasTablesIfNotExist(pqxx::connection * C);
			std::string toSqlTimeStamp(time_t time, pqxx::connection * C);

			bool isResultSaved(long long rId, const std::string & rUom, const std::string & rTableName, pqxx::connection * C);
			template <typename Meas> std::string getResultTableName();

			template <typename Meas> bool isMeasSaved( Meas * meas, pqxx::connection * C)
			{

				  if (!C->is_open()) {
					 std::cout << "isMeasSaved: Database not opened" << std::endl;
					 return false;
				  }

				  try
				  {

					  std::string phenomenonTime = toSqlTimeStamp(meas->phenomenonTime.time, C);
					  std::string resultTime = toSqlTimeStamp(meas->resultTime.time, C);

					  std::stringstream sqlSs;
					  sqlSs << "SELECT EXISTS(SELECT 1 FROM MEASUREMENTS WHERE ";
					  sqlSs << "IDENT = \'" << meas->identifier.value << "\' AND ";
					  sqlSs << "PROCEDURE = \'" << meas->procedure << "\' AND ";
					  sqlSs << "OBS_PROP = \'" << meas->observableProperty << "\' AND ";
					  sqlSs << "PHENOM_TIME = \'" << phenomenonTime << "\' AND ";
					  sqlSs << "RESULT_TIME = \'" << resultTime  << "\'";
					  sqlSs << ") AS \"EXISTS\";";

					  /* Create a non-transactional object. */
					  pqxx::nontransaction N(*C);
      
					  /* Execute SQL query */
					  pqxx::result R( N.exec( sqlSs ));
      
					  return R.begin()[0].as<bool>();
				 }
				 catch (const std::exception &e)
				 {
					std::cout << "isMeasSaved: " << e.what() << std::endl;
					return false;
				 }

			}

			template <typename Meas>
			long long getMeasId( Meas * meas, pqxx::connection * C)
			{
				if (!C->is_open()) 
				{
					std::cout << "getMeasId: Database not opened" << std::endl;
					return LLONG_MIN;
				}

				try
				{
					std::string phenomenonTime = toSqlTimeStamp(meas->phenomenonTime.time, C);
					std::string resultTime = toSqlTimeStamp(meas->resultTime.time, C);

					std::stringstream sqlSs;
					sqlSs << "SELECT ID FROM MEASUREMENTS WHERE ";
					sqlSs << "IDENT = \'" << meas->identifier.value << "\' AND ";
					sqlSs << "PROCEDURE = \'" << meas->procedure << "\' AND ";
					sqlSs << "OBS_PROP = \'" << meas->observableProperty << "\' AND ";
					sqlSs << "PHENOM_TIME = \'" << phenomenonTime << "\' AND ";
					sqlSs << "RESULT_TIME = \'" << resultTime << "\'";
					sqlSs << ";";

					/* Create a non-transactional object. */
					pqxx::nontransaction N(*C);
      
					/* Execute SQL query */
					pqxx::result R( N.exec( sqlSs ));

					if (R.begin() == R.end())
					{
						std::cout << "getMeasId: Assigned Measurement not found in DB" << std::endl;
						return LLONG_MIN;
					}
      
					return R.begin()[0].as<long long>();
				}
				catch (const std::exception &e)
				{
					std::cout << "getMeasId: " << e.what() << std::endl;
					return false;
				}
			}

			template <typename Meas>
			bool saveResult( Meas * meas, pqxx::connection * C)
			{
				if (!C->is_open()) 
				{
					std::cout << "saveResult: Database not opened" << std::endl;
					return false;
				}

				try
				{
					long long measId = getMeasId<Meas>(meas, C);

					if (LLONG_MIN == measId)
					{
						return false;
					}

					if (isResultSaved(measId, meas->result.uom, getResultTableName<Meas>(), C))
					{
						std::stringstream sqlSs;
						/* Create a transactional object. */
						pqxx::work W(*C);
						/* Create  SQL UPDATE statement */
						sqlSs << "UPDATE " << getResultTableName<Meas>() << " set RESULT = " << meas->result.toSqlString() << " ,";
						sqlSs << " UOM = \'" << meas->result.uom << "\'";
						sqlSs << " WHERE ID_MEAS = " << measId << ";";
						/* Execute SQL query */
						W.exec( sqlSs );
						W.commit();
						std::cout << "Result " << meas->result.valueToString() << " updated" << std::endl;
					}
					else
					{
						std::stringstream sqlSs;
						/* Create SQL statement */
						sqlSs << "INSERT INTO " << getResultTableName<Meas>() << "(ID_MEAS,UOM,RESULT) ";  
						sqlSs << "VALUES (" << measId << ",\'" << meas->result.uom << "\'," << meas->result.toSqlString() << "); ";

						/* Create a transactional object. */
						pqxx::work W(*C);
      
						/* Execute SQL query */
						W.exec( sqlSs );
						W.commit();
						std::cout << "Result " << meas->result.valueToString() << " saved" << std::endl;

					}
      
					return true;
				}
				catch (const std::exception &e)
				{
					std::cout << "saveResult: " << e.what() << std::endl;
					return false;
				}
			}

			template <typename Meas>
			bool saveMeas( Meas * meas, pqxx::connection * C)
			{
				if (!C->is_open()) {
						std::cout << "saveMeas: Database not opened" << std::endl;
						return false;
					}

					try
					{
					std::string phenomenonTime = toSqlTimeStamp(meas->phenomenonTime.time, C);
					std::string resultTime = toSqlTimeStamp(meas->resultTime.time, C);
		
					char * sql;
					sql = "INSERT INTO MEASUREMENTS" \
							"(" \
							"IDENT," \
							"IDENT_CS," \
							"PROCEDURE," \
							"OFFERING," \
							"OBS_PROP," \
							"FOI_IDENT," \
							"FOI_IDENT_CS," \
							"FOI_NAME," \
							"FOI_NAME_CS," \
							"FOI_SAMP_FE," \
							"FOI_GEOMETRY," \
							"PHENOM_TIME," \
							"RESULT_TIME," \
							"SAVE_TIME" \
							") VALUES (";

					std::stringstream sqlSs;
					sqlSs << sql;
					sqlSs << "\'" << meas->identifier.value << "\',";
					sqlSs << "\'" << meas->identifier.codespace << "\',";
					sqlSs << "\'" << meas->procedure << "\',";
					sqlSs << "\'" << meas->offering << "\',";
					sqlSs << "\'" << meas->observableProperty << "\',";
					sqlSs << "\'" << meas->featureOfInterest.identifier.value << "\',";
					sqlSs << "\'" << meas->featureOfInterest.identifier.codespace << "\',";
					sqlSs << "\'" << meas->featureOfInterest.name.value << "\',";
					sqlSs << "\'" << meas->featureOfInterest.identifier.codespace << "\',";
					sqlSs << "\'" << meas->featureOfInterest.sampledFeature << "\',";
					sqlSs << "ST_GeomFromText(\'" << meas->featureOfInterest.geometry.toSqlString() << "\', 4326),";
					sqlSs << "\'" << phenomenonTime << "\',";
					sqlSs << "\'" << resultTime << "\',";
					sqlSs << "\'now\');";

						/* Create a transactional object. */
					pqxx::work W(*C);
      
					/* Execute SQL query */
					W.exec( sqlSs );
					W.commit();

					return saveResult<Meas>(meas, C);

					}
					catch (const std::exception &e)
					{
					std::cout << "saveMeas: " << e.what() << std::endl;
					return false;
					}

			}

			template <typename Meas>
			bool updateMeas( Meas * meas, pqxx::connection * C)
			{
				if (!C->is_open()) {
						std::cout << "updateMeas: Database not opened" << std::endl;
						return false;
				}

				try
				{
					long long measId = getMeasId<Meas>(meas, C);

					if (LLONG_MIN == measId)
					{
						return false;
					}

					std::string phenomenonTime = toSqlTimeStamp(meas->phenomenonTime.time, C);
					std::string resultTime = toSqlTimeStamp(meas->resultTime.time, C);
		
					char * sql;
					sql = "UPDATE MEASUREMENTS set " \
							"(" \
							"IDENT," \
							"IDENT_CS," \
							"PROCEDURE," \
							"OFFERING," \
							"OBS_PROP," \
							"FOI_IDENT," \
							"FOI_IDENT_CS," \
							"FOI_NAME," \
							"FOI_NAME_CS," \
							"FOI_SAMP_FE," \
							"FOI_GEOMETRY," \
							"PHENOM_TIME," \
							"RESULT_TIME," \
							"SAVE_TIME" \
							") = (";

					std::stringstream sqlSs;
					sqlSs << sql;
					sqlSs << "\'" << meas->identifier.value << "\',";
					sqlSs << "\'" << meas->identifier.codespace << "\',";
					sqlSs << "\'" << meas->procedure << "\',";
					sqlSs << "\'" << meas->offering << "\',";
					sqlSs << "\'" << meas->observableProperty << "\',";
					sqlSs << "\'" << meas->featureOfInterest.identifier.value << "\',";
					sqlSs << "\'" << meas->featureOfInterest.identifier.codespace << "\',";
					sqlSs << "\'" << meas->featureOfInterest.name.value << "\',";
					sqlSs << "\'" << meas->featureOfInterest.identifier.codespace << "\',";
					sqlSs << "\'" << meas->featureOfInterest.sampledFeature << "\',";
					sqlSs << "ST_GeomFromText(\'" << meas->featureOfInterest.geometry.toSqlString() << "\', 4326),";
					sqlSs << "\'" << phenomenonTime << "\',";
					sqlSs << "\'" << resultTime << "\',";
					sqlSs << "\'now\')";

					sqlSs << " WHERE " << "ID = " << measId << ";";

						/* Create a transactional object. */
					pqxx::work W(*C);
      
					/* Execute SQL query */
					W.exec( sqlSs );
					W.commit();

					return saveResult<Meas>(meas, C);

					}
					catch (const std::exception &e)
					{
					std::cout << "updateMeas: " << e.what() << std::endl;
					return false;
					}

			}

			template <typename Meas>
			bool saveObsIntoDB(Json::Value & rObsJson, pqxx::connection * C, const std::string & obsIdent = "",const std::string & resultObsProp = "", const std::string & resultUom = "")
			{
				try
				{
					//pqxx::connection C("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432");
					if (!C->is_open()) 
					{
						std::cout << "Can't open database" << std::endl;
						return false;
					}


					Meas meas;
					bool success = meas.fromJson(rObsJson);

					if (!success)
					{
						std::cout << "Unable to parse json" << std::endl;
						return false;
					}

					// changing observation id according to parameter 
					if (!obsIdent.empty())
					{
						meas.identifier.value = obsIdent;
					}

					// changing observable property according to parameter (not converting now, just changing label)
					if (!resultObsProp.empty())
					{
						meas.observableProperty = resultObsProp;
					}

					// changing uom according to parameter (not converting now, just changing label)
					if (!resultUom.empty())
					{
						meas.result.uom = resultUom;
					}

					//test if value is saved
					if (isMeasSaved<Meas>(&meas, C))
					{
						success = updateMeas<Meas>(&meas, C);
					}
					else
					{
						success = saveMeas<Meas>(&meas, C);
					}

					//char * sql;

					// /* Create SQL statement */
				//   sql = "SELECT exists(select 1 from contact where id=12) AS \"exists\";";

				//    /* Create a non-transactional object. */
				//   pqxx::nontransaction N(*C);
				//   
				//   /* Execute SQL query */
				//   pqxx::result R( N.exec( sql ));
				//   
				//   /* List down all the records */
				//   for (pqxx::result::const_iterator r = R.begin(); r != R.end(); ++r) {
				//      cout << "ID = " << c[0].as<int>() << endl;
				//      cout << "Name = " << c[1].as<string>() << endl;
				//      cout << "Age = " << c[2].as<int>() << endl;
				//      cout << "Address = " << c[3].as<string>() << endl;
				//      cout << "Salary = " << c[4].as<float>() << endl;
				//   }
				//   cout << "Operation done successfully" << endl;


					//C.disconnect ();
				}
				catch (const std::exception &e)
				{
					std::cout << e.what() << std::endl;
					return false;
				}
				return true;
			}

			std::stringstream getOneObservationJson();

			int getObsFromServer(const std::string & rUrl, const std::string & rReqSosJson);

		
			std::atomic<bool> isConnected;
			CachingStream cs;

		};

		size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);
		int writer(char *data, size_t size, size_t nmemb, MobileNoise::SOS::CachingStream *cstream);

	} // namespace SOS
} // namespace MobileNoise