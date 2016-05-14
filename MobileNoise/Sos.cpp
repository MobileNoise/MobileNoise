#pragma once

#include "Sos.h"

namespace MobileNoise
{
	namespace SOS
	{

		

		Point::Point()
			: latitude()// = double();
			, longitude()// = double();
			, id(std::numeric_limits<long long>::lowest())
		{
	
		}

		Point::Point(double lat, double lon, const std::string & rSrid)
			: latitude(lat)
			, longitude(lon)
			, srid(rSrid)
			, id(std::numeric_limits<long long>::lowest())
		{
		}

		std::string Point::toWkt()
		{
			std::string output("POINT(" + std::to_string(longitude) + " " + std::to_string(latitude) + ")");
			return output;
		}

		std::string Point::getSrid()
		{
			if ((srid == "degrees_loc") || (srid == "degrees"))
			{
				return "4326";
			}
			return srid;
		}

		Json::Value Point::toJson()
		{
			Json::Value output;
			output["type"] = "Point";
			output["coordinates"].append(latitude);
			output["coordinates"].append(longitude);
			return output;
		}

		bool Point::fromJson(Json::Value & rJson)
		{
			auto str = std::string(rJson.get("type", "").asCString());

			if (std::string(rJson.get("type", "").asCString()) != "Point")
				return false;

			if ((rJson["coordinates"].isArray()) && (rJson["coordinates"].size() > 1))
			{
				latitude = rJson["coordinates"][0].asDouble();
				longitude = rJson["coordinates"][1].asDouble();

				return true;
			}
			else
			{
				return false;
			}
		}

		std::string Point::toSqlString()
		{
			return toWkt();
		}

		int Point::getDimensions()
		{
			if ((std::abs(latitude) > 90) || (std::abs(longitude) > 180))
			{
				return -1;
			}
			return 0;
		}

		Point &Point::operator = (const Point & pt)
		{
			latitude = pt.latitude;
			longitude = pt.longitude;
			srid = pt.srid;

			return *this;
		}

		Point &Point::operator += (const Point & pt)
		{
			latitude += pt.latitude;
			longitude += pt.longitude;

			return *this;
		}

		Point &Point::operator -= (const Point & pt)
		{
			latitude -= pt.latitude;
			longitude += pt.longitude;

			return *this;
		}

		Point &Point::operator += (const double f)
		{
			latitude += f;
			longitude += f;

			return *this;
		}

		Point &Point::operator -= (const double f)
		{
			latitude -= f;
			longitude -= f;

			return *this;
		}

		bool Point::operator == (const Point & pt)
		{
			return latitude == pt.latitude &&
			longitude == pt.longitude &&
			srid == pt.srid;
		}

		bool Point::operator != (const Point & pt)
		{
			return latitude != pt.latitude ||
			longitude != pt.longitude ||
			srid != pt.srid;
		}

		bool Point::operator == (const Point & pt) const
		{
			return latitude == pt.latitude &&
			longitude == pt.longitude &&
			srid == pt.srid;
		}

		bool Point::operator != (const Point & pt) const
		{
			return latitude != pt.latitude ||
			longitude != pt.longitude ||
			srid != pt.srid;
		}

		double Point::getDistTo_m(const Point & rPt)
		{
			if (*this == rPt)
			{
				return 0;
			}

			double t1 = latitude * DegToRad;
			double t2 = rPt.latitude * DegToRad;
			double dl = (rPt.longitude - longitude) * DegToRad;
			const double s1 = sin(t1);
			t1 = cos(t1);
			const double s2 = sin(t2);
			t2 = cos(t2);
			dl = cos(dl);
			dl = acos(s1*s2+t1*t2*dl);
			return dl*6371.0087714;
		}

		LineString::LineString()
			: id(std::numeric_limits<long long>::lowest())
		{
	
		}

		LineString::LineString(std::vector<Point> & rCoords, const std::string & srid)
			: id(std::numeric_limits<long long>::lowest())
		{
			coordinates = rCoords;

			if (!srid.empty())
			{
				for (auto point : coordinates)
				{
					point.srid = srid;
				}
			}
		}

		std::string LineString::toWkt()
		{
			std::stringstream output;
			output << "LINESTRING(";
			if (!coordinates.empty())
			{
				for (size_t i = 0; i < coordinates.size() - 1; i++)
				{
					output << coordinates[i].longitude << " " << coordinates[i].latitude << ",";
				}
				output << coordinates[coordinates.size() - 1].longitude << " " << coordinates[coordinates.size() - 1].latitude << ")";
			}
			return output.str();
		}

		std::string LineString::getSrid()
		{
			if (!coordinates.empty())
			{
				return coordinates[0].getSrid();
			}
			return std::string();
		}

		Json::Value LineString::toJson()
		{
			Json::Value output;
			output["type"] = "LineString";

			for (auto pt : coordinates)
			{
				Json::Value ptCoords;
				ptCoords.append(pt.longitude); // LONGITUDE FIRST, then Latitude
				ptCoords.append(pt.latitude);
				output["coordinates"].append(ptCoords);
			}

			if (!getSrid().empty())
			{
				output["crs"]["properties"]["name"] = getSrid();
			}
			return output;
		}

		bool LineString::fromJson(Json::Value & rJson)
		{
			//auto type = std::string(rJson.get("type", "").asString());

			if (std::string(rJson.get("type", "").asString()) != "LineString")
				return false;

			std::string crs;
			if (rJson.isMember("crs") && rJson["crs"].isMember("properties") && rJson["crs"]["properties"].isMember("name"))
			{
				crs = rJson["crs"]["properties"].get("name", "").asString();
			}

			if (!crs.empty())
			{
				std::vector<std::string> tokens;
				split(crs, ":", tokens);
				crs = tokens[tokens.size()-1];
			}

			if ((rJson["coordinates"].isArray()) && (rJson["coordinates"].size() > 1))
			{

				for (const Json::Value& c : rJson["coordinates"])  // iterate over "coordinates"
				{
					Point pt(c[1].asDouble(), c[0].asDouble(), crs); // in JSON is Lon-Lat, not Lat-Lon!!!

					coordinates.push_back(pt);
					//strcords.push_back(coordinates.asString());
				}

				return true;
			}
			else
			{
				return false;
			}
		}

		std::string LineString::toSqlString()
		{
			return toWkt();
		}

		int LineString::getDimensions()
		{
			if (coordinates.empty())
			{
				return -1;
			}

			for (auto pt : coordinates)
			{
				if (0 > pt.getDimensions())
				{
					return pt.getDimensions();
				}
			}

			if (1 == coordinates.size())
			{
				return 0;
			}

			return 1;
		}

		Envelope::Envelope()
			: id(std::numeric_limits<long long>::lowest())
		{

		}

		Envelope::Envelope(Point & rMaxVals, Point & rMinVals)
			: maxVals(rMaxVals)
			, minVals(rMinVals)
			, id(std::numeric_limits<long long>::lowest())
		{
			
		}
			
		Envelope::Envelope(double maxLat, double maxLon, double minLat, double minLon, const std::string & srid)
			: maxVals(Point(maxLat, maxLon, srid))
			, minVals(Point(minLat, minLon, srid))
			, id(std::numeric_limits<long long>::lowest())
		{

		}

		std::string Envelope::toWkt()
		{
			return std::string(std::to_string(minVals.longitude) + ", " + std::to_string(minVals.latitude) + ", " + 
				std::to_string(maxVals.longitude) + ", " + std::to_string(maxVals.latitude));
		}

		std::string Envelope::getSrid()
		{
			return maxVals.getSrid();
		}

		/*
		"type": "Polygon",
        "coordinates": [
          [
            [
              50,
              7
            ],
            [
              53,
              7
            ],
            [
              53,
              10
            ],
            [
              50,
              10
            ],
            [
              50,
              7
            ]
          ]
        ]
      }
	  */

		Json::Value Envelope::toJson()
		{
			Json::Value output;
			output["type"] = "Polygon";

			Json::Value nw;
			nw.append(minVals.latitude);
			nw.append(minVals.longitude);
			output["coordinates"].append(nw);

			Json::Value sw;
			sw.append(minVals.latitude);
			sw.append(maxVals.longitude);
			output["coordinates"].append(sw);

			Json::Value se;
			se.append(maxVals.latitude);
			se.append(maxVals.longitude);
			output["coordinates"].append(se);

			Json::Value ne;
			ne.append(maxVals.latitude);
			ne.append(minVals.longitude);
			output["coordinates"].append(ne);

			output["coordinates"].append(nw);

			return output;
		}

		bool Envelope::fromJson(Json::Value & rJson)
		{
			auto str = std::string(rJson.get("type", "").asCString());

			if (std::string(rJson.get("type", "").asCString()) != "Polygon")
				return false;
			if ((rJson["coordinates"].isArray()) && (rJson["coordinates"].size() > 4))
			{
				Json::Value point;

				minVals.latitude = DBL_MAX;
				minVals.longitude = DBL_MAX;
				maxVals.latitude = -DBL_MAX;
				maxVals.longitude = -DBL_MAX;

				for (int i = 0; i < 5; i++)
				{
					point = rJson["coordinates"][i];
					if ((point.isArray()) && (point.size() > 1))
					{
						double latitude = point[0].asDouble();
						double longitude = point[1].asDouble();

						if (minVals.latitude > latitude)
						{
							minVals.latitude = latitude;
						}

						if (minVals.longitude > longitude)
						{
							minVals.longitude = longitude;
						}

						if (maxVals.latitude < latitude)
						{
							maxVals.latitude = latitude;
						}

						if (maxVals.longitude < longitude)
						{
							maxVals.longitude = longitude;
						}
						
					}
					else
					{
						return false;
					}
				}
				return true;
			}
			return false;
		}

		std::string Envelope::toSqlString()
		{
			if (maxVals.srid.empty())
			{
				return "ST_MakeEnvelope(" + toWkt() + ")";
			}
			return "ST_MakeEnvelope(" + toWkt() + ", " + maxVals.srid + ")";
		}

		int Envelope::getDimensions()
		{
			int latDim = 1, lonDim = 1;
			if (maxVals.latitude == minVals.latitude)
			{
				latDim = 0;
			}
			if (maxVals.longitude == minVals.longitude)
			{
				lonDim = 0;
			}
			return latDim + lonDim;
		}

		void split(std::string str, std::string splitBy, std::vector<std::string>& tokens)
		{
			/* Store the original string in the array, so we can loop the rest
			 * of the algorithm. */
			tokens.push_back(str);

			// Store the split index in a 'size_t' (unsigned integer) type.
			size_t splitAt;
			// Store the size of what we're splicing out.
			size_t splitLen = splitBy.size();
			// Create a string for temporarily storing the fragment we're processing.
			std::string frag;
			// Loop infinitely - break is internal.
			while(true)
			{
				/* Store the last string in the vector, which is the only logical
				 * candidate for processing. */
				frag = tokens.back();
				/* The index where the split is. */
				splitAt = frag.find(splitBy);
				// If we didn't find a new split point...
				if(splitAt == std::string::npos)
				{
					// Break the loop and (implicitly) return.
					break;
				}
				/* Put everything from the left side of the split where the string
				 * being processed used to be. */
				tokens.back() = frag.substr(0, splitAt);
				/* Push everything from the right side of the split to the next empty
				 * index in the vector. */
				tokens.push_back(frag.substr(splitAt+splitLen, frag.size()-(splitAt+splitLen)));
			}
		}

		double set_snan()
		{
			double f;
			*((long long*)&f) = 0x7ff0000000000001;
			return f;
		}

		bool is_snan(double& f)
		{
		  return (*((long long*)&f) == 0x7ff0000000000001);
		}
	} // namespace SOS
} // namespace MobileNoise