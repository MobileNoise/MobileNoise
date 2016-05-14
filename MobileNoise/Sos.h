#pragma once
#pragma warning(disable:4996)

#define _USE_MATH_DEFINES

#include "SqlTransferable.h"
#include "json/json.h"

#include <math.h>
#include <vector>
#include <ctime>
#include <limits>

//#ifdef _MSC_VER
//#define INFINITY (DBL_MAX+DBL_MAX)
//#define NAN (INFINITY-INFINITY)
//#endif


namespace MobileNoise
{
	namespace SOS
	{

		const double DegToRad = M_PI / 180.0;
		const double RadToDeg = 180.0 / M_PI;

		double set_snan();

		bool is_snan(double& f);

		void split(std::string str, std::string splitBy, std::vector<std::string>& tokens);

		struct JsonTransferable
		{
			JsonTransferable(){}
			virtual ~JsonTransferable(){}
			virtual Json::Value toJson() = 0;
			virtual bool fromJson(Json::Value & rJson) = 0;
		};

		struct Geo
		{
			virtual std::string toWkt() = 0;
			virtual std::string getSrid() = 0;
			virtual int getDimensions() = 0;
		};

		struct Point : Geo, JsonTransferable, SqlTransferable
		{
			double latitude;
			double longitude;
			std::string srid;
			long long id;

			Point();
			Point(double lat, double lon, const std::string & srid);

			virtual std::string toWkt() override;
			virtual std::string getSrid() override;

			Json::Value toJson() override;
			bool fromJson(Json::Value & rJson) override;
			std::string toSqlString() override;
			int getDimensions() override;

			Point &Point::operator = (const Point & pt);
			Point &Point::operator += (const Point & pt);
			Point &Point::operator -= (const Point & pt);
			Point &Point::operator += (const double f);
			Point &Point::operator -= (const double f);
			bool Point::operator == (const Point & pt);
			bool Point::operator != (const Point & pt);
			bool Point::operator == (const Point & pt) const;
			bool Point::operator != (const Point & pt) const;

			double getDistTo_m(const Point & rPt);
		};

		struct LineString : Geo, JsonTransferable, SqlTransferable
		{
			std::vector<Point> coordinates;
			long long id;
			
			LineString();
			LineString(std::vector<Point> & rCoords, const std::string & srid = "");

			virtual std::string toWkt() override;
			virtual std::string getSrid() override;

			Json::Value toJson() override;
			bool fromJson(Json::Value & rJson) override;
			std::string toSqlString() override;
			int getDimensions() override;
		};

		struct Envelope : Geo, JsonTransferable, SqlTransferable
		{
			Point maxVals;
			Point minVals;
			long long id;

			Envelope();
			Envelope(Point & rMaxVals, Point & rMinVals);
			Envelope(double maxLat, double maxLon, double minLat, double minLon, const std::string & srid = "4326");

			virtual std::string toWkt() override;
			virtual std::string getSrid() override;

			Json::Value toJson() override;
			bool fromJson(Json::Value & rJson) override;
			std::string toSqlString() override;
			int getDimensions() override;
		};

		template <typename T> struct Geometry : JsonTransferable, SqlTransferable
		{
			T coordinates;

			Geometry<T>()
			{
				coordinates = T();
			}

			Json::Value toJson() override;
			bool fromJson(Json::Value & rJson) override;
			virtual std::string toSqlString() override;
		};

		template <> struct Geometry<Point> : Geo, JsonTransferable, SqlTransferable
		{
			Point coordinates;

			Json::Value toJson()
			{
				Json::Value output;
				output["coordinates"] = (coordinates.toJson());
				return output;
			}
			bool fromJson(Json::Value & rJson)
			{
				return coordinates.fromJson(rJson);
			}

			virtual std::string getSrid()
			{
				return coordinates.getSrid();
			}

			virtual std::string toWkt() override
			{
				return coordinates.toWkt();
			}

			virtual std::string toSqlString() override
			{
				return coordinates.toSqlString();
			}

			virtual int getDimensions() override
			{
				return coordinates.getDimensions();
			}

		};
		

		struct Identifier : JsonTransferable
		{
			std::string codespace;
			std::string value;

			Json::Value toJson() override
			{
				Json::Value output;
				output["codespace"] = codespace;
				output["value"] = value;
				return output;
			}

			bool fromJson(Json::Value & rJson) override
			{
				codespace = rJson.get("codespace", "").asString();
				value = rJson.get("value", "").asString();
				if (codespace.empty() || value.empty())
				{
					return false;
				}
				return true;
			}

		};

		template <typename T> struct Result : JsonTransferable, SqlTransferable
		{
			std::string uom;
			T value;

			Result()
			{
				value = T();
			}

			Result(T & rValue, std::string & rUom)
				: value(rValue)
				, uom(rUom)
			{
			}

			Json::Value toJson() override;
			bool fromJson(Json::Value & rJson) override;

			std::string valueToString();
			std::string toSqlString();

		};

		template <> struct Result<double> : JsonTransferable, SqlTransferable
		{
			std::string uom;
			double value;

			Result() : value(double())
			{
			}

			Result(double & rValue, std::string & rUom)
				: value(rValue)
				, uom(rUom)
			{
			}

			Json::Value toJson() override
			{
				Json::Value output;
				output["result"]["value"] = value;
				output["result"]["uom"] = uom;
				return output;
			}
			bool fromJson(Json::Value & rJson) override
			{
				uom = rJson.get("uom", "").asString();
				value = rJson.get("value", set_snan()).asDouble();
				return value != set_snan();
			}

			std::string valueToString()
			{
				return std::to_string(value);
			}

			std::string toSqlString() override
			{
				return valueToString();
			}

		};

		template <> struct Result<Point> : JsonTransferable, SqlTransferable
		{
			std::string uom;
			Point value;

			Result() : value(Point())
			{
			}

			Result(Point & rValue, std::string & rUom)
				: value(rValue)
				, uom(rUom)
			{
			}

			Json::Value toJson() override
			{
				Json::Value output;
				output["result"]["value"] = value.toJson();
				output["result"]["uom"] = uom;
				return output;
			}
			bool fromJson(Json::Value & rJson) override
			{
				uom = rJson.get("uom", "").asString();
				return value.fromJson(rJson);
			}

			std::string valueToString()
			{
				return value.toWkt();
			}

			std::string toSqlString() override
			{
				return std::string("ST_GeomFromText(\'" + valueToString() + "\', 4326)");
			}

		};

		struct Time
		{
			time_t time;

			static std::string toIsoStr(time_t rTime)
			{
				std::tm * timeinfo;
				timeinfo = gmtime(&rTime);

				return toIsoStr(timeinfo);
			}

			static std::string toIsoStr(std::tm * rTimeInfo)
			{
				char buffer[80];
				strftime(buffer,80,"%Y-%m-%dT%H:%M:%S", rTimeInfo);
				std::string str(buffer);
				str += ".000Z";

				return str;
			}

			std::string toIso()
			{
				return toIsoStr(time);
			}

			static std::tm fromIsoStr(const std::string & rTime)
			{
				std::tm time;
				sscanf(rTime.c_str(), "%04d-%02d-%02dT%02d:%02d:%02d", &time.tm_year, &time.tm_mon, &time.tm_mday, &time.tm_hour, &time.tm_min, &time.tm_sec);

				time.tm_year -= 1900;
				time.tm_mday -= 1;

				/*int tz = rTime.find("+");

				if (std::string::npos == tz)
				{
					tz = rTime.find("-");
				}
			
				
				if (std::string::npos != tz)
				{
					if (tz == 19)
					{
						std::string tzStr = rTime.substr(tz, std::string::npos);
						int tzH = 0;
						int tzM = 0;
						sscanf(tzStr.c_str(), "%02d:%02d", &tzH, &tzM);
						tzH = -tzH;
						if (tzH < 0)
						{
							tzM = -tzM;
						}

						time.tm_hour += tzH;
						time.tm_min += tzM;
					}

				}*/
				return time;
			}


			time_t fromIso(const std::string & rTime)
			{
				tm t = fromIsoStr(rTime);
				time = mktime(&t);
				return time;
			}

			Time() : time(0) {}
			Time(std::string & rISO)
			{
				fromIso(rISO);
			}
			Time(time_t rTime) : time(rTime)
			{
			}
		};

		template <typename T> struct FeatureOfInterest : JsonTransferable
		{
			Identifier identifier;
			Identifier name;
			std::string sampledFeature;
			Geometry<T> geometry;

			Json::Value toJson() override
			{
				Json::Value output;
				output["identifier"] = identifier.toJson();
				output["name"] = name.toJson();
				output["sampledFeature"] = sampledFeature;
				output["geometry"] = geometry.toJson();
				return output;
			}
			bool fromJson(Json::Value & rJson) override
			{
				bool success = true;
				success = identifier.fromJson(rJson["identifier"]);
				success = success && name.fromJson(rJson["name"]);
				sampledFeature = rJson["sampledFeature"].asString();
				success = success && geometry.fromJson(rJson["geometry"]);
				return success;
			}
		};

		template <typename T, typename U> struct OM_Measurement : JsonTransferable, SqlTransferable
		{
			Identifier identifier;
			std::string procedure;
			std::string offering;
			std::string observableProperty;
			FeatureOfInterest<U> featureOfInterest;
			Time phenomenonTime;
			Time resultTime;
			Result<T> result;


			Json::Value toJson() override
			{
				Json::Value output;
				output["identifier"] = identifier.toJson();
				output["procedure"] = procedure;
				output["offering"] = offering;
				output["observableProperty"] = observableProperty;
				output["featureOfInterest"] = featureOfInterest.toJson();
				output["phenomenonTime"] = phenomenonTime.toIso();
				output["resultTime"] = resultTime.toIso();
				output["result"] = result.toJson();
				return output;
			}
			bool fromJson(Json::Value & rJson) override
			{
				bool success = true;
				success = identifier.fromJson(rJson["identifier"]);
				procedure = rJson.get("procedure", "").asString();
				offering = rJson.get("offering", "").asString();
				observableProperty = rJson.get("observableProperty", "").asString();
				success = success && featureOfInterest.fromJson(rJson["featureOfInterest"]);
				phenomenonTime.fromIso(rJson.get("phenomenonTime", "").asString());
				resultTime.fromIso(rJson.get("resultTime", "").asString());
				success = success && result.fromJson(rJson["result"]);
				if (procedure.empty() || offering.empty() || observableProperty.empty() || !phenomenonTime.time || !resultTime.time)
					success = false;

				return success;
			}

			virtual ~OM_Measurement() override
			{

			}

			virtual std::string toSqlString() override
			{
				return std::string();
			}
		};

		template <typename T, typename U> struct Observations : JsonTransferable, SqlTransferable
		{
			std::vector<OM_Measurement<T,U>> observations;

			Json::Value toJson() override
			{
				Json::Value output;
				for (size_t i = 0; i < observations.size(); i++)
				{
					output.append(observations[i].toJson());
				}
				return output;
			}
			bool fromJson(Json::Value & rJson) override
			{
				bool success = true;
				for (size_t i = 0; i < rJson.size(); i++)
				{
					OM_Measurement<T> input;
					if (!input.fromJson(rJson[i]))
						success = false;
					observations.push_back(input);
				}
				return success;
			}
		};

		template <typename T> struct Request : JsonTransferable
		{
			std::string request;
			std::string version;
			std::string service;
			T data;

			Request<T>()
			{
				data = T();
			}

			Json::Value toJson() override
			{
				return Json::Value();
			}
			bool fromJson(Json::Value & rJson) override
			{
				return false;
			}
		};



	} // namespace SOS
} // namespace MobileNoise