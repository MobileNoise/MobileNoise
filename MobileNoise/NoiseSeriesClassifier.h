#include "NoiseClassifier.h"

namespace MobileNoise
{
	class NoiseSeriesClassifier 
	{
		// 1. make view of coords and ids of values according to time (or other criterion)
		// 2. get min and max coords values from view
		// 3. create partbounding boxes based on series, exclude empty
		// 4. for each partbounding box do - this can be multithraded
		//	1. read geographies from map
		//	2. classify and save each value
		// 5. clean memory after work

		//bool determineAffectedArea(const std::string & rDbConnStr, const std::string & rTableName, std::string & rViewName, const std::string & rCriterionColumn, const std::string & rStartCriterion, const std::string & rEndCriterion, MobileNoise::SOS::Point & rPartBoundingBoxSize, std::map<double, double> & rBoundingBoxes);
		//bool runClassification(rMapDbReadingFunction, rDataDbReadingFunction, rClassifyingFunction, const std::map<double, double> & rBoundingBoxes, const std::string & rBboxSrid, const std::string & rMapDbSrid, const std::string & rDataDbSrid);
		bool clean(const std::string & rDbConnStr, std::string & rViewName);

	};

	class NoiseMeterClassifier
	{
		// determines series for one device.
		bool makeSeries(time_t rMinInterval_s, const std::string & rDeviceName, const std::string & rTableName, pqxx::nontransaction * N, pqxx::work * W)
		{
			try
			{
				/*if (!C->is_open())
				{
						std::cout << "NoiseMeterClassifier::makeSeries: Database not opened" << std::endl;
						return false;
				}*/
				std::cout << "Making series for " << rDeviceName << " ... " << std::endl;

				long long lastSerId = 0;
				std::string last_phen_time_str;
				time_t last_phen_time;

				// select oldest time of not marked row
				std::stringstream sql;
				sql << "SELECT phenom_time FROM measurements WHERE procedure = \'" << rDeviceName << "\' AND series_id IS NULL ORDER BY phenom_time ASC LIMIT 1;";
				//pqxx::nontransaction N(*C);
				pqxx::result R( N->exec( sql ));
				auto row = R.begin();

				// if there is no unmarked row, all rows are marked, thus exit
				if (row == R.end())
				{
					std::cout << "WARNING: All measurments for device " << rDeviceName << " are already marked (or there should be NONE for it in the database).";
					return true;
				}


				std::string first_phen_time_str = row[0].as<std::string>();

				// get if there is older results, which are marked		
				sql = std::stringstream();
				sql << "SELECT EXISTS (SELECT 1 FROM measurements WHERE phenom_time < \'" << first_phen_time_str << "\' AND procedure = \'" << rDeviceName << "\' AND series_id IS NOT NULL);";
				//N = pqxx::nontransaction(*C);
				R = pqxx::result( N->exec( sql ));
				row = R.begin();
				bool isOlderResults = row[0].as<bool>();

				if (isOlderResults)
				{
					// get previous marked result of newest unmarked result
					sql = std::stringstream();
					sql << "SELECT series_id, phenom_time FROM measurements WHERE procedure = \'" << rDeviceName << "\' AND phenom_time < \'" << first_phen_time_str << "\' ORDER BY phenom_time DESC LIMIT 1;";
					//N = pqxx::nontransaction(*C);
					R = pqxx::result( N->exec( sql ));
					row = R.begin();
					lastSerId = row[0].as<long long>();

					last_phen_time_str = row[1].as<std::string>();
					last_phen_time = NoiseClassifier<pqxx::connection>::timestampConv(last_phen_time_str);
				}
				else
				{
					last_phen_time_str = first_phen_time_str;
					last_phen_time = NoiseClassifier<pqxx::connection>::timestampConv(last_phen_time_str);
					last_phen_time--;
				}

				// get number of rows to (re)process
				sql = std::stringstream();
				sql << "SELECT COUNT(id) FROM " << rTableName << " WHERE procedure = \'" << rDeviceName << "\' AND phenom_time >= \'" << first_phen_time_str << "\';";
				//N = pqxx::nontransaction(*C);
				R = pqxx::result( N->exec( sql ));
				row = R.begin();
				long long rows = row[0].as<long long>();
				size_t cycles = static_cast<size_t>((static_cast<double>(rows) / 100.0) + 1.0);

				/* Create a transactional object. */
				//pqxx::work W(*C);
			  
				// if there is more than 100 rows, work for every 100 rows
				for (size_t i = 0; i < cycles; i++)
				{
					// select given 100 max rows
					last_phen_time_str = NoiseClassifier<pqxx::connection>::timestampConv(last_phen_time);
					sql = std::stringstream();
					sql << "SELECT id, phenom_time FROM " << rTableName << " WHERE procedure = \'" << rDeviceName << "\' AND phenom_time > \'" << last_phen_time_str << "\' ORDER BY phenom_time ASC LIMIT 100;";
					//N = pqxx::nontransaction(*C);
					R = pqxx::result( N->exec( sql ));

					sql = std::stringstream();
					//W = pqxx::work(*C);

					 /* List down all the records */
					for (auto row = R.begin(); row != R.end(); ++row) 
					{
						long long id = row[0].as<long long>();
						std::string phen_time_str = row[1].as<std::string>();
						time_t phen_time = NoiseClassifier<pqxx::connection>::timestampConv(phen_time_str);

						if (rMinInterval_s < (phen_time - last_phen_time))
						{
							lastSerId++;
						}

						last_phen_time = phen_time;


						/* Create  SQL UPDATE statement */
						sql << "UPDATE measurements SET series_id = " << lastSerId << " WHERE id = " << id << "; ";
						foiGeomAsPosGeom(rDeviceName, rTableName, id, sql);
					}

					/* Execute SQL query */
					W->exec( sql );
					//W->commit();
				}

				return true;
			}
			catch (const std::exception &e)
			{
				std::cout << "makeSeries: " << e.what() << std::endl;
				return false;
			}


		}

		bool determineSeries(time_t rMinInterval, const std::string & rTableName, pqxx::nontransaction * N, pqxx::work * W)
		{
			try
			{
				/*if (!C->is_open())
				{
						std::cout << "NoiseMeterClassifier::determineSeries: Database not opened" << std::endl;
						return false;
				}*/

				bool success = true;

				std::stringstream sql;
				sql << "SELECT procedure FROM " << rTableName << " GROUP BY procedure;";
				//pqxx::nontransaction N(*C);
				pqxx::result R( N->exec( sql ));
				/* List down all the records */
				for (auto row = R.begin(); row != R.end(); ++row) 
				{
					std::string device = row[0].as<std::string>();

					success = makeSeries(rMinInterval, device, rTableName, N, W);

					if (!success)
					{
						return false;
					}

					/*success = foiGeomAsPosGeom(device, rTableName, W);

					if (!success)
					{
						return false;
					}*/
				}


			}
			catch (const std::exception &e)
			{
				std::cout << "determineSeries: " << e.what() << std::endl;
				return false;
			}
			return true;
		}

		bool foiGeomAsPosGeom(const std::string & rDeviceName, const std::string & rTableName, long long meas_id, std::stringstream & sql)
		{

			try
			{
				/*if (!C->is_open())
				{
						std::cout << "NoiseMeterClassifier::FoiGeomAsPosGeom: Database not opened" << std::endl;
						return false;
				}*/

				
				sql << "UPDATE " << rTableName << " SET foi_geometry = result ";
				sql << " FROM res_geom WHERE " << rTableName << ".id = id_meas ";
				sql << " AND id_meas = " << meas_id;
				sql << " AND procedure = \'" << rDeviceName << "\' AND uom = \'4326\';";

				/* Create a transactional object. */
				//pqxx::work W(*C);
      
				/* Execute SQL query */
				//W->exec( sql );
				//W->commit();

				return true;

			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseMeterClassifier::FoiGeomAsPosGeom: " << e.what() << std::endl;
				return false;
			}
		}

		public:
		bool runClassification(const std::string rMobNoiseConnStr,const std::string & rTableName, time_t rMinInterval)
		{
			try
			{
				
				pqxx::connection wC(rMobNoiseConnStr);
				if (!wC.is_open())
				{
						std::cout << "NoiseMeterClassifier::runClassification: Transactional connection not opened" << std::endl;
						return false;
				}
				pqxx::connection nC(rMobNoiseConnStr);
				if (!nC.is_open())
				{
						std::cout << "NoiseMeterClassifier::runClassification: Non-transactional connection not opened" << std::endl;
						return false;
				}

				pqxx::work W(wC);
				pqxx::nontransaction N (nC);

				bool success = determineSeries(rMinInterval, rTableName, &N, &W);

				W.commit();

				wC.disconnect();
				nC.disconnect();

				return success;
			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseMeterClassifier::FoiGeomAsPosGeom: " << e.what() << std::endl;
				return false;
			}
			
			
		}
	};
}