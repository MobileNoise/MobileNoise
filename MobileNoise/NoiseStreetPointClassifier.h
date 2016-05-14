#pragma once
#include <random>
#include <algorithm>
#include <mutex>
#include "NoiseClassifier.h"

namespace MobileNoise
{
	class NoiseStreetPointClassifier 
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

		struct PointLastDist
		{
			double dist;
			SOS::Point pt;
			long long index;
		};

		struct MultiLineStringPart
		{
			int id;
			SOS::LineString coordinates;
			int prevId;
			int nextId;

			MultiLineStringPart()
				: id (-1)
				, prevId(-1)
				, nextId(-1)
			{

			}

			MultiLineStringPart(SOS::LineString & rCoords)
				: id (-1)
				, prevId(-1)
				, nextId(-1)
				, coordinates(rCoords)
			{

			}
		};

		struct MultiLineString
		{
			long long id;
			std::map<int, MultiLineStringPart*> sortedParts;

			MultiLineString()
				: lastId(-1)
				, secondTry(false)
				, prevMin1(0)
				, nextMin1(0)
			{
			}

			void insertPart(MultiLineStringPart & rPart)
			{
				lastId++;
				rPart.id = lastId;
				parts.push_back(rPart);
			}

			bool sortParts(double rMaxDist_m)
			{
				sortedParts.clear();

				if (parts.size() == 1)
				{
					std::pair<int, MultiLineStringPart*> oneP(0, &(parts[0]));
					sortedParts.insert(oneP);
					return true;
				}


				for (auto part : parts)
				{
					if (!part.coordinates.coordinates.empty())
					{
						double minDist = std::numeric_limits<double>::max();
						//search for previous
						for (auto sub : parts)
						{
							if (part.id == sub.id)
							{
								continue;
							}

							if (!sub.coordinates.coordinates.empty())
							{
								double dist = part.coordinates.coordinates.begin()->getDistTo_m(*(sub.coordinates.coordinates.end()-1));
								if (dist < minDist)
								{
									minDist = dist;
									if (minDist <= rMaxDist_m)
									{
										part.prevId = sub.id;
										sub.nextId = part.id;
									}
								}
							}
						}
					}
				}

				// check wether all prevId is unique, except -1
				for (auto part : parts)
				{
					for (auto sub : parts)
					{
						if (part.id == sub.id)
						{
							continue;
						}

						if (part.prevId == sub.prevId)
						{
							if (!secondTry)
							{
								if (part.prevId != -1)
								{
									std::cout << "ERROR: For line osm_id " << this->id << " two subparts (" << part.id << ", " << sub.id 
										<< 	") have same previous subpart (" << part.prevId << "), trying lower rMaxDist_m parameter to "
										<< rMaxDist_m/2 << " ("
										<< "now is " << rMaxDist_m << ")." << std::endl;
									secondTry = true;
									prevMin1 = 0;
									nextMin1 = 0;
									bool success = sortParts(rMaxDist_m/2);
									if (success)
									{
										secondTry = false;
										prevMin1 = 0;
										nextMin1 = 0;
										return true;
									}
									std::cout << "ERROR: For line osm_id " << this->id << " two subparts (" << part.id << ", " << sub.id 
										<< 	") have same previous subpart (" << part.prevId << "), same result with lower rMaxDist_m parameter ("
										<< rMaxDist_m/2 << "). Aborting." << std::endl ;
									secondTry = false;
									prevMin1 = 0;
									nextMin1 = 0;
									return false;
								}
								else
								{
									if ((part.nextId == -1) || (sub.nextId == -1))
									{
										continue;
									}

									prevMin1++;
									
									if (prevMin1 > 1)
									{
										std::cout << "ERROR: For line osm_id " << this->id << " two subparts (" << part.id << ", " << sub.id 
										<< 	") have no previous subpart (" << part.prevId << "), trying higher rMaxDist_m parameter to "
										<< rMaxDist_m*2 << " ("
										<< "now is " << rMaxDist_m << ")." << std::endl ;
										secondTry = true;
										prevMin1 = 0;
										nextMin1 = 0;
										bool success = sortParts(rMaxDist_m*2);
										if (success)
										{
											secondTry = false;
											prevMin1 = 0;
											nextMin1 = 0;
											return true;
										}
										std::cout << "ERROR: For line osm_id " << this->id << " two subparts (" << part.id << ", " << sub.id 
											<< 	") have no previous subpart (" << part.prevId << "), same result with higher rMaxDist_m parameter ("
											<< rMaxDist_m*2 << "). Aborting." << std::endl ;
										secondTry = false;
										prevMin1 = 0;
										nextMin1 = 0;
										return false;
									}
								}
							}
							else // if second try
							{
								if ((part.prevId != -1) || (prevMin1 > 1))
								{
									return false;
								}
							}
						}
						
						if (part.nextId == sub.nextId)
						{
							if (!secondTry)
							{
								if (part.nextId != -1)
								{
									std::cout << "ERROR: For line osm_id " << this->id << " two subparts (" << part.id << ", " << sub.id 
										<< 	") have same next subpart (" << part.nextId << "), trying lower rMaxDist_m parameter to "
										<< rMaxDist_m/2 << " ("
										<< "now is " << rMaxDist_m << ")." << std::endl ;
									secondTry = true;
									bool success = sortParts(rMaxDist_m/2);
									if (success)
									{
										secondTry = false;
										return true;
									}
									std::cout << "ERROR: For line osm_id " << this->id << " two subparts (" << part.id << ", " << sub.id 
										<< 	") have same next subpart (" << part.nextId << "), same result with lower rMaxDist_m parameter ("
										<< "now is " << rMaxDist_m/2 << "). Aborting." << std::endl ;
									secondTry = false;
									prevMin1 = 0;
									nextMin1 = 0;
									return false;
								}
								else
								{
									if ((part.prevId == -1) || (sub.prevId == -1))
									{
										continue;
									}

									nextMin1++;
									
									if (nextMin1 > 1)
									{
										std::cout << "ERROR: For line osm_id " << this->id << " two subparts (" << part.id << ", " << sub.id 
										<< 	") have no next subpart (" << part.nextId << "), trying higher rMaxDist_m parameter to "
										<< rMaxDist_m*2 << " ("
										<< "now is " << rMaxDist_m << ")." << std::endl ;
										secondTry = true;
										prevMin1 = 0;
										nextMin1 = 0;
										bool success = sortParts(rMaxDist_m*2);
										if (success)
										{
											secondTry = false;
											prevMin1 = 0;
											nextMin1 = 0;
											return true;
										}
										std::cout << "ERROR: For line osm_id " << this->id << " two subparts (" << part.id << ", " << sub.id 
											<< 	") have no previous subpart (" << part.prevId << "), same result with higher rMaxDist_m parameter ("
											<< "now is " << rMaxDist_m << "). Aborting." << std::endl ;
										secondTry = false;
										prevMin1 = 0;
										nextMin1 = 0;
										return false;
									}
								}
							}
							else
							{
								if ((part.nextId != -1) || (nextMin1 > 1))
								{
									return false;
								}
							}
						}
					}
				}

				for (auto part : parts)
				{
					if ((part.prevId != -1) && (part.nextId != -1))
					{
						std::pair<int, MultiLineStringPart*> oneP(part.prevId, &part);
						sortedParts.insert(oneP);
					}
					
				}
				secondTry = false;
				prevMin1 = 0;
				nextMin1 = 0;

				return true;
			}

			bool getSortedLine(SOS::LineString & rSortedLine, double rMaxDist_m)
			{
				if (!sortParts(rMaxDist_m))
				{
					return false;
				}

				rSortedLine.coordinates.clear();

				for (auto part : sortedParts)
				{
					auto coords = part.second->coordinates.coordinates;
					for (auto pt : coords)
					{
						rSortedLine.coordinates.push_back(pt);
					}
				}
				return true;
			}

		protected:
			std::vector<MultiLineStringPart> parts;
			int lastId;
			bool secondTry;
			int prevMin1;
			int nextMin1;
		};


		bool runClassification
			( const std::string & rNoiseDbConnStr
			, const std::string & rOsmDbConnStr
			, const std::string & rNoiseDataTableName
			, const std::string & rGeoDataTableName
			, const std::string & rNoiseDataViewName
			, const std::string & rNoiseDataSaveStartTime
			, const std::string & rNoiseDataSaveEndTime
			, const std::string & rOsmIdToIndexTableName
			, const std::string & rNoiseStreetPointTableName
			, const std::string & rNoiseStreetSidePointTableName
			, double rMaxSpan_m
			, size_t rCycleSize
			, std::multimap<long long, std::string> & rNotIndexedLines
			, std::mutex & rNotIndexedLinesMutex
			, std::multimap<std::string, long long> & rNotIndexedSeries
			, double rMinSameLinePointDist_m
			, double rMinAnotherLinePointDist_m
			)
		{
			// cycle above each serie and prepare data for (re)indexation
			prepareSerieValuesToIndexation
				( rNoiseDbConnStr
				, rOsmDbConnStr
				, rNoiseDataTableName
				, rGeoDataTableName
				, rNoiseDataViewName
				, rNoiseDataSaveStartTime
				, rNoiseDataSaveEndTime
				, rOsmIdToIndexTableName
				, rNoiseStreetPointTableName
				, rNoiseStreetSidePointTableName
				, rMaxSpan_m
				, rCycleSize
				, rNotIndexedLines
				, rNotIndexedLinesMutex
				, rNotIndexedSeries
				, rMinSameLinePointDist_m
				, rMinAnotherLinePointDist_m
				);

			// index all noise points in whole batch to be sequencable in order
			// use just osm_ids successfully touched in given batch, not other.
			createIndexOfNoiseLinePoints;
			
			// note point sideing according to azimuth between first and last point of line. If they are identical, left is on left. Write down max and min azimuths for leftness.

			// for every point 
			// 1. if not out: find lines within given distance
			// 2. a. if one line, assign to given line
			//	  b. else 
			//		1. get two nearest points on each line with distance at least x, measure azimuth between, normalize to 0 - 180
			//		2. get azimuth between this series point and previous series point which is in at least x distance, this and next , thist and next next , for first point from next next next
			//		make mean azimuth, normalize to 0 - 180
			//		3. which azimuth difference to series is minimal, then assign to.
			//      4. if there are no points to end, get them also from previous, if possible
			// 3. assign means: copy pt id, procedure id, phentime, Laeq, compute distance and uncert to left, center or right part

		}


		// cycle above each serie and prepare data for (re)indexation
		bool prepareSerieValuesToIndexation
			( const std::string & rNoiseDbConnStr
			, const std::string & rOsmDbConnStr
			, const std::string & rNoiseDataTableName
			, const std::string & rGeoDataTableName
			, const std::string & rNoiseDataViewName
			, const std::string & rNoiseDataSaveStartTime
			, const std::string & rNoiseDataSaveEndTime
			, const std::string & rOsmIdToIndexTableName
			, const std::string & rNoiseStreetPointTableName
			, const std::string & rNoiseStreetSidePointTableName
			, double rMaxSpan_m
			, size_t rCycleSize
			, std::multimap<long long, std::string> & rNotIndexedLines
			, std::mutex & rNotIndexedLinesMutex
			, std::multimap<std::string, long long> & rNotIndexedSeries
			, double rMinSameLinePointDist_m
			, double rMinAnotherLinePointDist_m
			)
		{
			try
			{
				pqxx::connection cNoise(rNoiseDbConnStr);

				if (!cNoise.is_open())
				{
						std::cout << "NoiseStreetPointClassifier::prepareSerieValuesToIndexation: Noise database not opened" << std::endl;
						return false;
				}

				pqxx::connection cGeo(rOsmDbConnStr);

				if (!cGeo.is_open())
				{
						std::cout << "NoiseStreetPointClassifier::prepareSerieValuesToIndexation: Geo database not opened" << std::endl;
						return false;
				}

				// Get all new measurements: measurements within given save_time criteria and save them as given new view
				std::stringstream sql;
				sql << "CREATE OR REPLACE VIEW " << rNoiseDataViewName << " AS SELECT * FROM " << rNoiseDataTableName;
				sql << " WHERE save_time > \'" << rNoiseDataSaveStartTime << "\' AND save_time < \'" << rNoiseDataSaveEndTime << "\';";

				/* Create a transactional object. */
				pqxx::work W(cNoise);
      
				/* Execute SQL query */
				W.exec( sql );
				W.commit();

				// Get list of all unique pairs of devices and series over that view
				clearss( sql );
				sql << "SELECT procedure, series_id FROM " << rNoiseDataViewName << " GROUP BY procedure, series_id ORDER BY procedure, series_id; ";

				pqxx::nontransaction N(cNoise);
				pqxx::result R( N.exec( sql ));

				// For every pair prepare data for indexation
				for (auto row = R.begin(); row != R.end(); ++row) 
				{
					std::string device = row[0].as<std::string>();
					long long serieId = row[1].as<long long>();

					prepareNoiseTableForIndexing
						( rNoiseDbConnStr
						, rOsmDbConnStr
						, rNoiseDataViewName
						, rGeoDataTableName
						, device
						, rOsmIdToIndexTableName
						, serieId
						, rNoiseStreetPointTableName 
						, rNoiseStreetSidePointTableName
						, rMaxSpan_m
						, rCycleSize
						, rNotIndexedLines
						, rNotIndexedLinesMutex
						, rNotIndexedSeries
						, rMinSameLinePointDist_m
						, rMinAnotherLinePointDist_m
						);


				}
				

				return true;
			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::prepareSerieValuesToIndexation: ERROR " << e.what() << std::endl;
				return false;
			}

		}

		// cycle above each serie is above this function, this function works on one serie.
		// computing point indexes and sideing, and all other computation should be worked after this function
		bool prepareNoiseTableForIndexing
			( const std::string & rNoiseDbConnStr
			, const std::string & rOsmDbConnStr
			, const std::string & rNoiseDataViewName
			, const std::string & rGeoDataTableName
			, const std::string & rDeviceName
			, const std::string & rOsmIdToIndexTableName
			, const long long rSerieId
			, const std::string & rNoiseStreetPointTableName
			, const std::string & rNoiseStreetSidePointTableName
			, double rMaxSpan_m
			, size_t rCycleSize
			, std::multimap<long long, std::string> & rNotIndexedLines
			, std::mutex & rNotIndexedLinesMutex
			, std::multimap<std::string, long long> & rNotIndexedSeries
			, double rMinSameLinePointDist_m
			, double rMinAnotherLinePointDist_m
			)
		{
			try
			{
				pqxx::connection cNoise(rNoiseDbConnStr);
				if (!cNoise.is_open())
				{
						std::cout << "NoiseStreetPointClassifier::getNearestLines: Noise database not opened" << std::endl;
						return false;
				}
				pqxx::connection cGeo(rOsmDbConnStr);
				if (!cGeo.is_open())
				{
						std::cout << "NoiseStreetPointClassifier::getNearestLines: Geo database not opened" << std::endl;
						return false;
				}

			
				std::string bboxViewName;

				// create random name of view;
				std::default_random_engine generator;
				std::uniform_int_distribution<int> distribution(1000000000,9999999999);
				int dice_roll = distribution(generator); 

				bboxViewName = std::to_string(rSerieId) + "_" + std::to_string(dice_roll);


				//create bounding box of given serie and save it into db
				std::stringstream sql;
				sql << "SELECT ST_SetSRID(ST_Extent(" << rNoiseDataViewName << ".foi_geometry),4326) AS bbox INTO " << bboxViewName << "_bbox FROM " << rNoiseDataViewName;
				sql << " WHERE procedure = \'" << rDeviceName << "\' AND series_id = \'" << rSerieId << "\';";

				// create view of given serie points with random name inside bbox
				sql << "CREATE OR REPLACE VIEW " << bboxViewName << " AS SELECT * FROM " << rNoiseDataViewName  << " WHERE procedure = \'" << rDeviceName << "\' AND series_id = \'" << rSerieId << "\';";

				/* Create a transactional object. */
				pqxx::work W(cNoise);
      
				/* Execute SQL query */
				W.exec( sql );
				W.commit();

				// get the boundingbox coords
				sql = std::stringstream();
				sql << "SELECT ST_AsEWKT(bbox) FROM " << bboxViewName << "_bbox LIMIT 1;";
				pqxx::nontransaction N(cNoise);
				pqxx::result R( N.exec( sql ));
				auto row = R.begin();
				std::string bbox = row[0].as<std::string>();

				// expand bounding box by ca 10 m and save to geo database
				sql = std::stringstream();
				sql << "SELECT ST_Transform(ST_Expand(ST_GeomFromEWKT(\'" << bbox << "\'),0.0001),900913) AS bbox INTO " << bboxViewName << "_geobbox;";

				// create intersect of lines from geodatabase according to bbox, they will have osm id and geometry
				sql << "SELECT clipped.osm_id, clipped_geom INTO " << bboxViewName << "_geo ";
				sql << "FROM (SELECT " << rGeoDataTableName << ".osm_id, (ST_Dump(ST_Intersection(" << bboxViewName << "_geobbox.bbox, " << rGeoDataTableName << ".way))).geom As clipped_geom ";
				sql << "FROM " << bboxViewName << "_geobbox ";
				sql << "INNER JOIN " << rGeoDataTableName << " ";
				sql << "ON ST_Intersects(" << bboxViewName << "_geobbox.bbox, " << rGeoDataTableName << ".way))  As clipped ";
				sql << "WHERE ST_Dimension(clipped.clipped_geom) > 0 ;";

				// create id for rows in that table
				sql << "ALTER TABLE " << bboxViewName << "_geo ADD COLUMN id SERIAL PRIMARY KEY;";

				// transform to WGS84
				sql << "ALTER TABLE " << bboxViewName << "_geo ALTER COLUMN clipped_geom TYPE Geometry(Linestring, 4326) USING ST_Transform(clipped_geom, 4326);";

				//sql << "SELECT * FROM " << rGeoDataViewName << " INTO " << bboxViewName << "_geo WHERE " << rGeoDataViewName << ".way && ST_Transform(ST_Expand(ST_GeomFromEWKT(" << bbox << "),0.0001),900913);";
				/* Create a transactional object. */
				W = pqxx::work(cGeo);
      
				/* Execute SQL query */
				W.exec( sql );
				W.commit();

				std::pair<std::string, long long> pa(rDeviceName, rSerieId);


				// create table with streetpoints in noise db
				if (!createTableOfStreetPoints(&cNoise, rNoiseStreetPointTableName, rNoiseStreetSidePointTableName))
				{
					std::cout << "NoiseStreetPointClassifier::getNearestLines: Not able to write to database, skipping device " << rDeviceName << " serie " << rSerieId << std::endl;
					rNotIndexedSeries.insert(pa);
					return false;
				}

				// for every selected line, for given part in inside bbox, create points.
				// if point is near another point: if same oid, nothing to do, if another, make link
				if (!prepareLinePoints(&cNoise, &cGeo, rNoiseDataViewName, bboxViewName + "_geo", rNoiseStreetPointTableName, rNoiseStreetSidePointTableName, bboxViewName, rMaxSpan_m, rCycleSize, rNotIndexedLines, rNotIndexedLinesMutex, rMinSameLinePointDist_m, rMinAnotherLinePointDist_m))
				{
					std::cout << "ERROR: Noise points for device " << rDeviceName << " serie " << rSerieId << " not prepared. This serie will be skipped during computation! " << std::endl;
					rNotIndexedSeries.insert(pa);
					return false;
				}

				// insert all osm_ids which was updated by prepareLinePoints to given table to be indexed in next classifier step
				if (!saveOsmIdToIndexation(&cNoise, rOsmIdToIndexTableName, rNotIndexedLines, rNotIndexedLinesMutex))
				{
					std::cout << "ERROR: Unable to save data to table for indexation, serie " << rSerieId << " of device " << rDeviceName << " will be skipped during computation! " << std::endl;
					rNotIndexedSeries.insert(pa);
					return false;
				}

				// clear all temporary created views 
				if (!clearTempViews(&cNoise, &cGeo, bboxViewName))
				{
					std::cout << "ERROR: Unable to clear temporary tables with prefix " << bboxViewName << " for serie " << rSerieId << " of device " << rDeviceName << ", please delete them by hand. " << std::endl;
				}
			

			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::getNearestLines: ERROR " << e.what() << std::endl;
				return false;
			}
			return true;
		}

		bool clearTempViews(pqxx::connection * cNoise, pqxx::connection * cGeo, const std::string & rBboxViewName)
		{
			try
			{
				//pqxx::connection cNoise(rNoiseDbConnStr);
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::clearTempViews: ERROR Noise database not opened" << std::endl;
				}
				else
				{

					std::stringstream sql;

					/* Create a transactional object. */
					pqxx::work W(*cNoise);

					sql <<  "DROP TABLE " << rBboxViewName << "_bbox; ";
					sql <<  "DROP VIEW " << rBboxViewName << ";" ;
      
					/* Execute SQL query */
					W.exec( sql );
					W.commit();
					
				}

			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::getNearestLines: NoiseDB ERROR " << e.what() << std::endl;
				//return false;
			}

			try
			{
				//pqxx::connection cGeo(rOsmDbConnStr);
				if (!cGeo->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::clearTempViews: Geo database not opened" << std::endl;
						return false;
				}


				std::stringstream sql;

				/* Create a transactional object. */
				pqxx::work W(*cGeo);

				sql <<  "DROP TABLE " << rBboxViewName << "_geobbox; ";
				sql <<  "DROP TABLE " << rBboxViewName << "_geo;" ;
      
				/* Execute SQL query */
				W.exec( sql );
				W.commit();

			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::getNearestLines: GeoDB ERROR " << e.what() << std::endl;
				return false;
			}
		}

		bool saveOsmIdToIndexation(pqxx::connection * cNoise, const std::string & rOsmIdToIndexTableName, std::multimap<long long, std::string> &  rNotIndexedLines, std::mutex & rNotIndexedLinesMutex)
		{

			try
			{
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::getNearestLines: Noise database not opened" << std::endl;
						return false;
				}

				std::stringstream sql;

				sql << "CREATE TABLE IF NOT EXISTS " << rOsmIdToIndexTableName << "("  ;
				sql << "ID			SERIAL UNIQUE PRIMARY KEY NOT NULL	," ;
				sql << "OSM_ID		BIGINT					  NOT NULL	," ;
				sql << "BBOX_ID		TEXT							    ," ;
				sql << "SAVE_TIME	TIMESTAMP				  NOT NULL	," ;
				sql << "); ";

				/* Create a transactional object. */
				pqxx::work W(*cNoise);
      
				/* Execute SQL query */
				W.exec( sql );
				W.commit();


				sql = std::stringstream();
				std::multimap<long long, std::string> toDelete;
				
				sql << "BEGIN; ";
				sql << "LOCK TABLE " << rOsmIdToIndexTableName << " IN SHARE ROW EXCLUSIVE MODE; ";

				{ // scope for lock
					std::unique_lock<std::mutex> lck(rNotIndexedLinesMutex);
					for (auto pair : rNotIndexedLines)
					{
						sql << "INSERT INTO " << rOsmIdToIndexTableName << " (osm_id ,bbox_id , save_time) ";
						sql	<< " SELECT " << pair.first << ", \'" << pair.second << "\', now ";
						sql << " WHERE NOT EXISTS (";
						sql << " SELECT id FROM " << rOsmIdToIndexTableName << " WHERE osm_id = " << pair.first;
						sql << " AND bbox_id = \'" << pair.second << "\' ";
						sql << "); ";
        
						toDelete.insert(pair);
					}
				}

				sql << " COMMIT; ";

				/* Create a transactional object. */
				W = pqxx::work(*cNoise);
      
				/* Execute SQL query */
				W.exec( sql );
				W.commit();

				// if success delete given key value pairs from not indexed lines multimap 
				// TODO make it faster
				for (auto toDelP : toDelete)
				{
					std::unique_lock<std::mutex> lck(rNotIndexedLinesMutex);
					std::pair<std::multimap<long long,std::string>::iterator,std::multimap<long long,std::string>::iterator> i;
					std::multimap<long long, std::string>::iterator j;
					i = rNotIndexedLines.equal_range(toDelP.first);

					for(j=i.first;j != i.second;++j)
					{
						if((j->second) == toDelP.second) {
							rNotIndexedLines.erase(j);
							break;
						}
					}
				}

			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::getNearestLines: " << e.what() << std::endl;
				return false;
			}
		}

		bool prepareLinePoints
			(pqxx::connection * cNoise
			, pqxx::connection * cGeo
			, const std::string & rNoiseDataViewName
			, const std::string & rGeoDataViewName
			, const std::string & rNoiseStreetPointsTableName
			, const std::string & rNoiseStreetPointsSideTableName
			, const std::string & rBboxTableName
			, double rMaxSpan_m
			, size_t rCycleSize
			, std::multimap<long long, std::string> & rNotIndexedLines
			, std::mutex & rNotIndexedLinesMutex
			, double rMinSameLinePointDist_m
			, double rMinAnotherLinePointDist_m)
		{
			try
			{
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::prepareLinePoints: Noise database not opened" << std::endl;
						return false;
				}

				if (!cGeo->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::prepareLinePoints: Geo database not opened" << std::endl;
						return false;
				}

				std::stringstream sql;
				sql << "SELECT COUNT(clipped_geom) FROM " << rGeoDataViewName << ";";
				pqxx::nontransaction N(*cGeo);
				pqxx::result R( N.exec( sql ));
				auto row = R.begin();
				long long rows = row[0].as<long long>();
				size_t cycles = static_cast<size_t>((static_cast<double>(rows) / static_cast<double>(rCycleSize)) + 1.0);

				/* Create a transactional object. */
				pqxx::work W(*cGeo);
				long long lastId = -1;
				std::map<long long, int> previousOsmIds;
			  
				// Create points on these lines according to criteria, work partially for every x lines
				for (size_t i = 0; i < cycles; i++)
				{
					sql = std::stringstream();
					sql << "SELECT osm_id, ST_AsGeoJSON((ST_Segmentize(clipped_geom::geography, " << rMaxSpan_m << ")),15, 2), id FROM " << rGeoDataViewName << " WHERE id > " << lastId << " ORDER BY id ASC LIMIT " << rCycleSize <<";";
					N = pqxx::nontransaction(*cGeo);
					R = pqxx::result( N.exec( sql ));

					
					 /* List down all the records */
					for (auto row = R.begin(); row != R.end(); ++row) 
					{
						lastId = row[2].as<long long>();
						std::stringstream linestr;
						linestr << row[1].as<std::string>();
						Json::Value lineJson;
						linestr >> lineJson;
						long long osmId = row[0].as<long long>();

						// get number of previous segments with same osm_id
						int sameOsmJumps = 1;

						auto idVal = previousOsmIds.find(osmId);
						if ( idVal == previousOsmIds.end() ) 
						{
							std::pair<long long, int> idRow(osmId, sameOsmJumps);
							previousOsmIds.insert(idRow);
						} 
						else 
						{
							idVal->second++;
							sameOsmJumps = idVal->second;
						}



						if (!createLinePoints(cNoise, rNoiseDataViewName, rBboxTableName, rNoiseStreetPointsTableName, rNoiseStreetPointsSideTableName, lineJson, rNotIndexedLines, rNotIndexedLinesMutex, osmId, rMinSameLinePointDist_m, rMinAnotherLinePointDist_m, sameOsmJumps))
						{
							std::cout << "ERROR: Unable to create noise line points for osm segment " << osmId << " no computations on this line will be performed." << std::endl;
							idVal->second = sameOsmJumps;
							continue;
						}
						idVal->second = sameOsmJumps;

					}

					sql = std::stringstream();
					W = pqxx::work(*cGeo);

					/* Execute SQL query */
					W.exec( sql );
					W.commit();
				}
			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::prepareLinePoints: " << e.what() << std::endl;
				return false;
			}
		}

		bool createLinePoints
			( pqxx::connection * cNoise
			, const std::string & rNoiseDataViewName
			, const std::string & rBboxTableName
			, const std::string & rNoiseStreetPointsTableName
			, const std::string & rNoiseStreetPointsSideTableName
			, Json::Value & rLineJson
			, std::multimap<long long, std::string> & rNotIndexedLines
			, std::mutex & rNotIndexedLinesMutex
			, long long rOsmId
			, double rMinSameLinePointDist_m
			, double rMinAnotherLinePointDist_m
			, int & rPreviousOsmJumps
			)
		{

			try
			{
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::createLinePoints: Noise database not opened" << std::endl;
						return false;
				}

				long long index = -1;
				long long previousIndex = -1;

				std::stringstream sql;
				SOS::LineString line;
				line.fromJson(rLineJson); //TODO TEST IT!

				std::pair<long long, std::string> nip(rOsmId,rBboxTableName);
				{ // scope for lock
					std::unique_lock<std::mutex> lck(rNotIndexedLinesMutex);
					rNotIndexedLines.insert(nip);
				}
				// For every point on line search  
				for (auto pt : line.coordinates)
				{
					// compute pt's coordinates in WGS84
					sql = std::stringstream();
					sql << "SELECT ST_AsText(ST_Transform(ST_GeometryFromText(\'" << pt.toWkt() << "\', " << pt.getSrid() << "), 4326));";
					pqxx::nontransaction N(*cNoise);
					pqxx::result R( N.exec( sql ));
					auto row = R.begin();
					std::string ptCoords = row[0].as<std::string>();

					//if there is another point with same street osm_id in given vicinity
					
					sql = std::stringstream();
					sql << "SELECT id, index FROM " << rNoiseStreetPointsTableName << " WHERE ST_DWithin(" << rNoiseStreetPointsTableName << ".coords::geography, ST_GeogFromText(\'" << ptCoords << "\'), " << rMinSameLinePointDist_m << ") ";
					sql << " AND  " << rNoiseStreetPointsTableName << ".osm_id = " << rOsmId << "; ";
					N = pqxx::nontransaction(*cNoise);
					R = pqxx::result( N.exec( sql ));
					
					bool isPt = R.begin() != R.end();

					// if is, nothing to do
					if (isPt)
					{
						row = R.begin();
						previousIndex = row[1].as<long long>();
						continue;
					}

					//if not, if there is another point with another osm_id in given vicinity

				    sql = std::stringstream();
					sql << "SELECT id FROM " << rNoiseStreetPointsTableName << " WHERE ST_DWithin(" << rNoiseStreetPointsTableName << ".coords::geography, ST_GeogFromText(\'" << ptCoords << "\'), " << rMinAnotherLinePointDist_m << ") ";
					sql << " AND  " << rNoiseStreetPointsTableName << ".osm_id != " << rOsmId << "; ";
					sql << "ORDER BY  " << rNoiseStreetPointsTableName << ".coords::geometry <-> ST_GeogFromText(\'" << ptCoords << "\')::geometry LIMIT 1;";
					N = pqxx::nontransaction(*cNoise);
					R = pqxx::result( N.exec( sql ));


					sql = std::stringstream();
					// if is, save link to that point

					if (R.begin() != R.end())
					{
						auto row = R.begin();
						long long pointId = row[0].as<long long>();

						// find if this value is already saved
						sql << "SELECT EXISTS (SELECT id FROM " << rNoiseStreetPointsTableName << " WHERE link = " << pointId;
						sql << " AND  " << rNoiseStreetPointsTableName << ".osm_id = " << rOsmId << "); ";
						pqxx::nontransaction Ne(*cNoise);
						pqxx::result Re( Ne.exec( sql ));
						row = Re.begin();
						bool isPt = row[0].as<bool>();
						sql = std::stringstream();

						// if is, nothing to do
						if (isPt)
						{
							continue;
						}

						// if not, save link

						sql << "INSERT INTO " << rNoiseStreetPointsTableName << "(osm_id, link, index, prev_index) VALUES (" << rOsmId << "," << pointId << "," << index << "," << previousIndex << ");";
						if (previousIndex >= 0)
						{
							rPreviousOsmJumps++;
							previousIndex = index = -1000000000 * rPreviousOsmJumps;
						}
						else
						{
							previousIndex = index;
							index--;
						}
						
					}
					//if not, save point
					else
					{
						sql << "INSERT INTO " << rNoiseStreetPointsTableName << "(osm_id, coords, index, prev_index) VALUES (" << rOsmId << ",ST_GeogFromText(\'" << ptCoords << "\')," << index << "," << previousIndex << ");";
						if (previousIndex >= 0)
						{
							rPreviousOsmJumps++;
							previousIndex = index = -1000000000 * rPreviousOsmJumps;
						}
						else
						{
							previousIndex = index;
							index--;
						}
						
					}

					/* Create a transactional object. */
					pqxx::work W(*cNoise);
      
					/* Execute SQL query */
					W.exec( sql );
					W.commit();

				}
				return true;
			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::createLinePoints: ERROR " << e.what() << std::endl;
				return false;
			}
		}

		bool createTableOfStreetPoints(pqxx::connection * cNoise, const std::string & rStreetPointsTableName, const std::string & rStreetPointsSideTableName)
		{
			try
			{

				//pqxx::connection C("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432");
				if (!cNoise->is_open()) 
				{
					std::cout << "NoiseStreetPointClassifier::createTableOfStreetPoints: Can't open database" << std::endl;
					return false;
				}

				std::stringstream sql;

				// -- MEASUREMENTS

				/* Create SQL statement */
				sql << "CREATE TABLE IF NOT EXISTS " << rStreetPointsTableName << "(" ;
				sql << "ID			SERIAL UNIQUE PRIMARY KEY NOT NULL," ;
				sql << "OSM_ID					BIGINT			    ," ;
				sql << "LINK					BIGINT			    ," ;
				sql << "INDEX					BIGINT			    ," ;
				sql << "PREV_INDEX				BIGINT			    ," ;
				sql << "COORDS					GEOMETRY			," ;
				sql << "AZIMUTH_RAD				REAL				," ;
				sql << "SECONDH_UNC				REAL				," ;
				sql << "SECONDH_LAEQ			REAL				," ;
				sql << "FOURTHH_UNC				REAL				," ;
				sql << "FOURTHH_LAEQ			REAL				," ;
				sql << "SIXTHH_UNC				REAL				," ;
				sql << "SIXTHH_LAEQ				REAL				," ;
				sql << "EIGHTHH_UNC				REAL				," ;
				sql << "EIGHTHH_LAEQ			REAL				," ;
				sql << "TENTHH_UNC				REAL				," ;
				sql << "TENTHH_LAEQ				REAL				," ;
				sql << "TWELWETHH_UNC			REAL				," ;
				sql << "TWELWETHH_LAEQ			REAL				," ;
				sql << "FOURTEENTHH_UNC			REAL				," ;
				sql << "FOURTEENTHH_LAEQ		REAL				," ;
				sql << "SIXTEENTHH_UNC			REAL				," ;
				sql << "SIXTEENTHH_LAEQ			REAL				," ;
				sql << "EIGHTTEENTHH_UNC		REAL				," ;
				sql << "EIGHTTEENTHH_LAEQ		REAL				," ;
				sql << "TWENTHH_UNC				REAL				," ;
				sql << "TWENTHH_LAEQ			REAL				," ;
				sql << "TWENTYSECONDTHH_UNC		REAL				," ;
				sql << "TWENTYSECONDTHH_LAEQ	REAL				," ;
				sql << "TWENTYFOURTHH_UNC		REAL				," ;
				sql << "TWENTYFOURTHH_LAEQ		REAL				," ;
				sql << "DAY_UNC					REAL				," ;
				sql << "DAY_LAEQ				REAL				," ;
				sql << "NIGHT_UNC				REAL				," ;
				sql << "NIGHT_LAEQ				REAL				," ;
				sql << "MEAN_UNC				REAL				," ;
				sql << "MEAN_LAEQ				REAL				";
				sql << "); " ;

				sql << "CREATE TABLE IF NOT EXISTS " << rStreetPointsSideTableName << "("  ;
				sql << "ID			SERIAL UNIQUE PRIMARY KEY NOT NULL	," ;
				sql << "OSM_ID		BIGINT								," ;
				sql << "STRP_ID	BIGINT	REFERENCES STREETPOINTS(ID) NOT NULL ," ;
				sql << "PROCEDURE	TEXT								," ;
				sql << "PHEN_TIME	TIMESTAMP				  NOT NULL	," ;
				sql << "SIDE		INT						  NOT NULL	," ;
				sql << "LAEQ_DB		REAL					  NOT NULL	," ;
				sql << "DIST_M		REAL					  NOT NULL	," ;
				sql << "UNC_DB		REAL					  NOT NULL	," ;
				sql << "); ";

	  
				/* Create a transactional object. */
				pqxx::work W(*cNoise);
      
				/* Execute SQL query */
				W.exec( sql );
				W.commit();
			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::createTableOfStreetPoints: ERROR " << e.what() << std::endl;
				return false;
			}
		}

//----------------------------------------------------

		bool createIndexOfNoiseLinePoints
			( pqxx::connection * cNoise
			, pqxx::connection * cGeo
			, const std::string & rNoiseStreetPointsTableName
			, const std::string & rGeoLineTableName
			, const std::string & rOsmIdToIndexTableName
			, std::map<long long, std::string> & rNotIndexedLines
			, std::mutex & rNotIndexedLinesMutex
			, double rSortingDist_m
			, double rMinSameLinePointDist_m
			)
		{
			try
			{
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::createIndexAndOrientOfNoiseLinePoints: Noise database not opened" << std::endl;
						return false;
				}

				if (!cGeo->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::createIndexAndOrientOfNoiseLinePoints: Geo database not opened" << std::endl;
						return false;
				}

				// get list of all osm ids

				std::stringstream sql;
				sql << "SELECT osm_id FROM " << rOsmIdToIndexTableName << " GROUP BY osm_id;";
				pqxx::nontransaction N(*cGeo);
				pqxx::result R( N.exec( sql ));

				// for every osm_id 
				for (auto row = R.begin(); row != R.end(); ++row) 
				{
					// read osm_id
					long long osm_id = row[0].as<long long>();

					// create multilinestring
					MultiLineString multiLine;
					multiLine.id = osm_id;

					// get all parts of one osm_id
					sql = std::stringstream();
					sql << "SELECT ST_AsGeoJSON(ST_Transform(way), 4326) AS geom FROM " << rGeoLineTableName << "  WHERE osm_id = " << osm_id << "; ";
					pqxx::nontransaction Ne(*cGeo);
					pqxx::result Re( Ne.exec( sql ));
					
					// for every part of given osm_id linestring

					for (auto lsRow = Re.begin(); lsRow != Re.end(); ++lsRow) 
					{
						std::stringstream ssJson;
						ssJson << lsRow[0].as<std::string>();
						Json::Value json;
						ssJson >> json;
			
						MultiLineStringPart lsp;
						lsp.coordinates.fromJson(json);

						multiLine.insertPart(lsp);
					}

					// connect more line segments if necessary
					// and get resulting line
					SOS::LineString osmLine;
					if (!multiLine.getSortedLine(osmLine,rSortingDist_m))
					{
						std::cout << "ERROR: Unable to sort line osm_id " << osm_id << ", skipping." << std::endl; 
						continue;
					}

					// get begins of all line segments (non indexed & indexed; non indexed first)
					std::map<int, SOS::Point> noiseLineSegBegPts;
					if (!getBeginsOfNoiseLineSegments(cNoise, rNoiseStreetPointsTableName, osm_id, noiseLineSegBegPts))
					{
						std::cout << "ERROR: Unable to get begin points of noise line segments for osm_id " << osm_id << ", skipping." << std::endl; 
						continue;
					}

					// create index of begins all noise points on this geo line (with all segments with same osm_id)
					std::map<size_t, PointLastDist> indexedBeginsMap;
					if (! createIndexOfBeginPointsAccordingToLine(noiseLineSegBegPts, osmLine, indexedBeginsMap))
					{
						std::cout << "ERROR: Unable to create index of noise line segment begin points for osm_id " << osm_id << ", skipping." << std::endl; 
						continue;
					}

					// create index of all noise points according to given segment begins (orientation of all segments is same)
					if (!saveIndexOfOidNoisePoints(cNoise, rNoiseStreetPointsTableName, osm_id, indexedBeginsMap, rMinSameLinePointDist_m ))
					{
						std::cout << "ERROR: Unable to create index of noise line segment points for osm_id " << osm_id << ", some points should not have right index." << std::endl; 
						std::cout << "WARNING: Results for line osm_id " << osm_id << " will be skipped in other computations!" << std::endl;
						continue;
					}

					// compute azimuth of all noise points (where is not computed) on osm_id
					// TODO update computing to give to account last param: rMinSameLinePointDist_m
					if (!computeAzimuthOfNoisePoints(cNoise, rNoiseStreetPointsTableName, osm_id, rMinSameLinePointDist_m))
					{
						std::cout << "ERROR: Unable to compute azimuth of noise line segment points for osm_id " << osm_id << ", some points should not have right azimuth." << std::endl; 
						std::cout << "WARNING: Results for line osm_id " << osm_id << " will be skipped in other computations!" << std::endl;
						continue;
					}

					{ // scope for lock
						std::unique_lock<std::mutex> lck(rNotIndexedLinesMutex);
						rNotIndexedLines.erase(osm_id); // erase all values with this osm_id
					}

					// erase osm_id value from table "to be indexed"
					if (!eraseOsmIdFromOsmIdToIndexTable(cNoise, rOsmIdToIndexTableName, osm_id))
					{
						std::cout << "ERROR: Unable to erase osm_id " << osm_id << " from table of lines to be indexed (despite the line was indexed successfully), line probably will be indexed again in next run." << std::endl; 
						std::cout << "WARNING: Results for line osm_id " << osm_id << " will be skipped in other computations!" << std::endl;
					}
				}

			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::createIndexAndOrientOfNoiseLinePoints: ERROR " << e.what() << std::endl;
				return false;
			}
		}

		bool getBeginsOfNoiseLineSegments(pqxx::connection * cNoise, const std::string & rNoiseStreetPointsTableName, long long rOsmId, std::map<int, SOS::Point> & rBeginPtsMap)
		{
			try
			{
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::getBeginsOfNoiseLineSegments: Noise database not opened" << std::endl;
						return false;
				}

				rBeginPtsMap.clear();

				// get all newly indexed point begins
				std::stringstream sql;
				sql << "SELECT index, ST_AsGeoJSON(coords), id FROM " << rNoiseStreetPointsTableName << " WHERE osm_id = " << rOsmId << " AND index = prev_index AND index < 0;";
				pqxx::nontransaction N(*cNoise);
				pqxx::result R( N.exec( sql ));

				for (auto row = R.begin(); row != R.end(); ++row) 
				{
					std::stringstream ssJson;
					ssJson << row[1].as<std::string>();
					Json::Value json;
					ssJson >> json;
					SOS::Point pt;
					pt.fromJson(json);
					pt.id = row[2].as<long long>();

					std::pair<int, SOS::Point> ptPair(row[0].as<int>(), pt);
					rBeginPtsMap.insert(ptPair);

				}

				// get all previously indexed point begins

				// select only points which have index bigger by more than 999999000 as their prev_index
				sql = std::stringstream();
				sql << "SELECT index, ST_AsGeoJSON(coords), id  FROM " << rNoiseStreetPointsTableName << " WHERE osm_id = " << rOsmId << " AND index > 0 AND (index > (prev_index + 1000) OR prev_index = 0);";
				N = pqxx::nontransaction(*cNoise);
				R = pqxx::result( N.exec( sql ));

				for (auto row = R.begin(); row != R.end(); ++row) 
				{
					std::stringstream ssJson;
					ssJson << row[1].as<std::string>();
					Json::Value json;
					ssJson >> json;
					SOS::Point pt;
					pt.fromJson(json);
					pt.id = row[2].as<long long>();

					std::pair<int, SOS::Point> ptPair(row[0].as<int>(), pt);
					rBeginPtsMap.insert(ptPair);

				}
				
				return true;

			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::getBeginsOfNoiseLineSegments: ERROR " << e.what() << std::endl;
				return false;
			}
		}

		bool createIndexOfBeginPointsAccordingToLine(const std::map<int, SOS::Point> & rPoints, const SOS::LineString & rLine, std::map<size_t, PointLastDist> & rIndexedBeginsMap)
		{
			if (rLine.coordinates.empty())
			{
				std::cout << "NoiseStreetPointClassifier::createIndexOfPointsAccordingToLine: ERROR: Input linestring empty" << std::endl;
				return false;
			}

			if (rPoints.empty())
			{
				std::cout << "NoiseStreetPointClassifier::createIndexOfPointsAccordingToLine: ERROR: Input points map empty" << std::endl;
				return false;
			}

			// if there is one point in whole line, abort for now
			if (rLine.coordinates.size() == 1)
			{
				std::cout << "NoiseStreetPointClassifier::createIndexOfPointsAccordingToLine: ERROR: Not implemented for just one pont of Input linestring" << std::endl;
				return false;
			}

			std::multimap<int, PointLastDist> indexedPoints;


			// for every point determine nearest point on line
			for (auto pt : rPoints)
			{
				double lastDist = std::numeric_limits<double>::max();
				int nearPtInd = 0;

				for (size_t i = 0; i < rLine.coordinates.size(); i++)
				{
					double dist = pt.second.getDistTo_m(rLine.coordinates[i]);
					if (dist < lastDist)
					{
						nearPtInd = i;
					}
				}

				PointLastDist ptl;
				ptl.dist = lastDist;
				ptl.pt = pt.second;
				ptl.index = pt.first;
				std::pair<int, PointLastDist>  ptp(nearPtInd, ptl);
				indexedPoints.insert(ptp);
			}

			// create hard sequence of points, if duplicity in indexes determine by distance to this and next pt.

			// get vector of unique keys
			std::vector<std::pair<int,PointLastDist>> keys_dedup;
			std::unique_copy(std::begin(indexedPoints),
              std::end(indexedPoints),
              back_inserter(keys_dedup),
              [](const std::pair<int,PointLastDist> &entry1,
                 const std::pair<int,PointLastDist> &entry2) 
				{
                   return (entry1.first == entry2.first);
				}
             );

			 size_t index = 0;
			 rIndexedBeginsMap.clear();

			 // for every key find if is unique
			 for (auto key : keys_dedup)
			 {
				int size = indexedPoints.count(key.first);

				// if is, save point with row index.
				if (1 == size)
				{
					std::pair<size_t, PointLastDist> pt(index, key.second);
					rIndexedBeginsMap.insert(pt);
					index++;
					continue;
				}

				std::multimap<double,PointLastDist> sameKeyVals;

				// if is not unique, get all values and determine distance to next point
				std::pair <std::multimap<int,PointLastDist>::iterator, std::multimap<int,PointLastDist>::iterator> ret;
				ret = indexedPoints.equal_range(key.first);
				for (std::multimap<int,PointLastDist>::iterator it=ret.first; it!=ret.second; ++it)
				{
					std::pair<double, PointLastDist> pa(DBL_MAX, it->second);
					sameKeyVals.insert(pa);
				}

				int add = 0;

				// if given point is not last point of the line,
				if (rLine.coordinates.size() - 1 > key.first)
				{
					// determine sequence according distance to next point of the line.
					add = 1;
				}
				// if is last, determine according previous point of the line
				else
				{
					add = -1;
				}
				
				// determine distance to previous / next point			
				for (auto pt : sameKeyVals)
				{
					double dist = pt.second.pt.getDistTo_m(rLine.coordinates[key.first + add]);
					std::pair<double, PointLastDist> val(dist, pt.second);
					sameKeyVals.insert(val);
				}

				// delete values with dbl_max
				sameKeyVals.erase(DBL_MAX);

				if (add < 0)
				{
					// insert in given order if computing from previous point dist
					for (auto pt : sameKeyVals)
					{
						std::pair<size_t, PointLastDist> pt(index, key.second);
						rIndexedBeginsMap.insert(pt);
						index++;
					}
				}
				else
				{
					// insert in reverse order if computing from next point dist
					for (auto it = sameKeyVals.rbegin(); it != sameKeyVals.rend(); it++)
					{
						std::pair<size_t, PointLastDist> pt(index, key.second);
						rIndexedBeginsMap.insert(pt);
						index++;
					}
				}
			 }
			 return true;
		}

		bool saveIndexOfOidNoisePoints(pqxx::connection * cNoise, const std::string & rNoiseStreetPointsTableName, long long rOsmId, std::map<size_t, PointLastDist> & rIndexedBeginsMap, double rMinSameLinePointDist_m )
		{
			try
			{
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::saveIndexOfOidNoisePoints: Noise database not opened" << std::endl;
						return false;
				}

				size_t lastInd = 0;

				//for all begin points determine rest of noise line segments 
				for (auto bpt : rIndexedBeginsMap)
				{
					long long lastId = 0;
					
					// get all points id from given segment
					std::stringstream sql;
					// if point's index is negative, get points between with index between point's index and point's index - 999999000
					if (bpt.second.index < 0)
					{
						sql << "SELECT id FROM " << rNoiseStreetPointsTableName << " WHERE osm_id = " << rOsmId << " AND index <= " << bpt.second.index << " AND index > " << bpt.second.index - 999999000 << ";";
					}
					// if point's index is positive (or 0!), get points with index between point's index and point's index + 999999000
					else
					{
						sql << "SELECT id FROM " << rNoiseStreetPointsTableName << " WHERE osm_id = " << rOsmId << " AND index >= " << bpt.second.index << " AND index < " << bpt.second.index + 999999000 << ";";
					}

					pqxx::nontransaction N(*cNoise);
					pqxx::result R( N.exec( sql ));

					//check wether previous segment is near this segment;
					bool inSequence = true;

					if ( lastId != 0)
					{
						sql = std::stringstream();
						sql << "SELECT ST_DWithin((SELECT coords FROM " << rNoiseStreetPointsTableName << " WHERE id = " << R.begin()[0].as<long long>() << " LIMIT 1)::geography, (SELECT coords FROM " << rNoiseStreetPointsTableName << " WHERE id = " << lastId << " LIMIT 1), " << rMinSameLinePointDist_m + (rMinSameLinePointDist_m/10) << ", false); ";
						pqxx::nontransaction Ne(*cNoise);
						pqxx::result Re( Ne.exec( sql ));
						
						inSequence = Re.begin()[0].as<bool>();
					}
					
					sql = std::stringstream();

					// Every point update with new index
					for (auto row = R.begin(); row != R.end(); ++row) 
					{
						double index;
						if (inSequence)
						{
							index = lastInd + 1;
						}
						else
						{
							index = ((lastInd / 1000000000) * 1000000000) + 1;
							inSequence = true;
						}

						lastId = row[0].as<long long>();
						sql << "UPDATE " << rNoiseStreetPointsTableName << " SET (index, prev_index) = (" << index << ", " << lastInd << ") WHERE id = " << lastId << "; ";
						lastInd = index + 1;
					}
					pqxx::work W(*cNoise);
					W.exec( sql );
					W.commit();
				}

				return true;
			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::saveIndexOfOidNoisePoints: ERROR " << e.what() << std::endl;
				return false;
			}
		}

		bool computeAzimuthOfNoisePoints(pqxx::connection * cNoise, const std::string & rNoiseStreetPointsTableName, long long rOsmId, double rMinSameLinePointDist_m )
		{
			try
			{
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::computeAzimuthOfPoints: Noise database not opened" << std::endl;
						return false;
				}

				std::stringstream sql;
				sql << "SELECT id, link, index, prev_index, azimuth_rad FROM " << rNoiseStreetPointsTableName;
				sql << " WHERE osm_id = " << rOsmId << " AND index IS NOT NULL AND prev_index IS NOT NULL ORDER BY index ASC; ";
				pqxx::nontransaction N(*cNoise);
				pqxx::result R( N.exec( sql ));

				if (R.begin() == R.end())
				{
					std::cout << "Computing azimuth of Noise Points: No result for osm_id " << rOsmId << std::endl;
					return false;
				}

				long long prevId = 0;

				pqxx::work W(*cNoise);

				for (auto row = R.begin(); row != R.end(); row++)
				{

					long long id = row[0].as<long long>();

					// skip if all three values are computed, or two values if begin or end or one value at all;
					if ((prevId == 0) && ((row+1) == R.end()))
					{
						std::cout << "WARNING: For line osm_id " << rOsmId << " there is only one point" << std::endl;
						return false;
					}
					else if ((prevId == 0) && (!row[4].is_null()) && (!(row+1)[4].is_null()))
					{
						prevId = id;
						continue;
					}
					else if (((row+1) == R.end()) && (!row[4].is_null()) && (!(row-1)[4].is_null()))
					{
						prevId = id;
						continue;
					}
					else if (((row+1)[4].is_null()) && (!row[4].is_null()) && (!(row-1)[4].is_null()))
					{
						prevId = id;
						continue;
					}


					clearss(sql);
					long long index = row[2].as<long long>();
					long long prevIndex = row[3].as<long long>();

					long long idWithGeom;
					if (row[1].is_null())
					{
						idWithGeom = id;
					}
					else
					{
						idWithGeom = row[1].as<long long>();
					}

					sql << " UPDATE "  << rNoiseStreetPointsTableName <<  " SET azimuth_rad  = " << 
					sql << " (SELECT ST_Azimuth ((";

					// from point
					//		on begin  OR	on begin of own segment
					if ((prevId == 0) || ((index - 1000) > prevIndex ))
					{
						sql << "SELECT coords FROM " << rNoiseStreetPointsTableName << " WHERE id = " << idWithGeom << " ";
					}
					else
					{
						long long prevIdWithGeom;
						if ((row-1)[1].is_null())
						{
							prevIdWithGeom = (row-1)[0].as<long long>();
						}
						else
						{
							prevIdWithGeom = (row-1)[1].as<long long>();
						}


						sql << "SELECT coords FROM " << rNoiseStreetPointsTableName << " WHERE id = " << prevIdWithGeom << " ";
					}
					
					sql << ")::geography,(";

					// to point
					//	this is last point   OR		this is last point of own segment
					if (((row+1) == R.end()) || (((row+1)[2].as<long long>() - 1000) > index ))
					{
						sql << "SELECT coords FROM " << rNoiseStreetPointsTableName << " WHERE id = " << idWithGeom << " ";
					}
					else
					{
						long long nextIdWithGeom;
						if ((row+1)[1].is_null())
						{
							nextIdWithGeom = (row+1)[0].as<long long>();
						}
						else
						{
							nextIdWithGeom = (row+1)[1].as<long long>();
						}

						sql << "SELECT coords FROM " << rNoiseStreetPointsTableName << " WHERE id = " << nextIdWithGeom << " ";
					}
					sql << ")::geography)) WHERE id = " << id << "; ";
					
					W.exec( sql );
					
					prevId = id;

				}

				W.commit();
				/*
				sql << "ID			SERIAL UNIQUE PRIMARY KEY NOT NULL," ;
				sql << "OSM_ID					BIGINT			    ," ;
				sql << "LINK					BIGINT			    ," ;
				sql << "INDEX					BIGINT			    ," ;
				sql << "PREV_INDEX				BIGINT			    ," ;
				sql << "COORDS					GEOMETRY			," ;
				sql << "AZIMUTH_RAD				REAL				," ;
				*/


				return true;
			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::computeAzimuthOfPoints: ERROR " << e.what() << std::endl;
				return false;
			}
		}

		bool eraseOsmIdFromOsmIdToIndexTable(pqxx::connection * cNoise, const std::string & rOsmIdToIndexTableName, long long rOsmId)
		{
			try
			{
				if (!cNoise->is_open())
				{
						std::cout << "NoiseStreetPointClassifier::eraseOsmIdFromToIndexTable: Noise database not opened" << std::endl;
						return false;
				}

				std::stringstream sql;
				sql << "DELETE FROM " << rOsmIdToIndexTableName << " WHERE osm_id = " << rOsmId << ";";
				pqxx::work W(*cNoise);
				W.exec( sql );
				W.commit();

				return true;
			}
			catch (const std::exception &e)
			{
				std::cout << "NoiseStreetPointClassifier::eraseOsmIdFromToIndexTable: ERROR " << e.what() << std::endl;
				return false;
			}

		}


		static void clearss(std::stringstream & ss)
		{
			ss.str(std::string());
			ss.clear();
			return;
		}

	};
}