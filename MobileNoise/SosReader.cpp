#pragma once

#include "SosReader.h"

namespace MobileNoise
{
	namespace SOS
	{

		SosReader::SosReader()
			: isConnected(false)
 
		{

		}

		bool SosReader::createMeasTablesIfNotExist(pqxx::connection * C)
		{
			try
			{
				//pqxx::connection C("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432");
				if (C->is_open()) 
				{
					std::cout << "Opened database successfully: " << C->dbname() << std::endl;
				} 
				else 
				{
					std::cout << "Can't open database" << std::endl;
					return false;
				}

				char * sql;

				// -- MEASUREMENTS

				/* Create SQL statement */
				sql = "CREATE TABLE IF NOT EXISTS MEASUREMENTS("  \
				"ID SERIAL UNIQUE PRIMARY KEY NOT NULL," \
				"SERIES_ID		INT					 ," \
				"IDENT			TEXT		NOT NULL," \
				"IDENT_CS       TEXT				," \
				"PROCEDURE      TEXT		NOT NULL," \
				"OFFERING		  TEXT				," \
				"OBS_PROP		  TEXT				," \
				"FOI_IDENT	  TEXT		NOT NULL," \
				"FOI_IDENT_CS	  TEXT				," \
				"FOI_NAME		  TEXT				," \
				"FOI_NAME_CS	  TEXT				," \
				"FOI_SAMP_FE	  TEXT		NOT NULL," \
				"FOI_GEOMETRY	  GEOMETRY			," \
				"PHENOM_TIME	  TIMESTAMP NOT NULL," \
				"RESULT_TIME	  TIMESTAMP NOT NULL," \
				"SAVE_TIME		  TIMESTAMP NOT NULL" \
				");";

	  
				/* Create a transactional object. */
				pqxx::work W(*C);
      
				/* Execute SQL query */
				W.exec( sql );
				W.commit();
				std::cout << "Table MEASUREMENTS in the database" << std::endl;

				// -- RES_REAL

				sql = "CREATE TABLE IF NOT EXISTS RES_REAL("  \
				"ID SERIAL UNIQUE PRIMARY KEY				NOT NULL," \
				"ID_MEAS INT REFERENCES MEASUREMENTS(ID)	NOT NULL," \
				"UOM			  TEXT								," \
				"RESULT	      REAL						NOT NULL" \
				");";
      
				/* Execute SQL query */
				pqxx::work Wa(*C);
				Wa.exec( sql );
				Wa.commit();
				std::cout << "Table RES_REAL in the database" << std::endl;

				// -- RES_INT

				sql = "CREATE TABLE IF NOT EXISTS RES_INT("  \
				"ID SERIAL UNIQUE PRIMARY KEY				NOT NULL," \
				"ID_MEAS INT REFERENCES MEASUREMENTS(ID)	NOT NULL," \
				"UOM			  TEXT								," \
				"RESULT	      INT						NOT NULL" \
				");";
      
				/* Execute SQL query */
				pqxx::work Wb(*C);
				Wb.exec( sql );
				Wb.commit();
				std::cout << "Table RES_INT in the database" << std::endl;

				// -- RES_TEXT

				sql = "CREATE TABLE IF NOT EXISTS RES_TEXT("  \
				"ID SERIAL UNIQUE PRIMARY KEY				NOT NULL," \
				"ID_MEAS INT REFERENCES MEASUREMENTS(ID)	NOT NULL," \
				"UOM			  TEXT								," \
				"RESULT	      TEXT						NOT NULL" \
				");";
      
				/* Execute SQL query */
				pqxx::work Wc(*C);
				Wc.exec( sql );
				Wc.commit();
				std::cout << "Table RES_TEXT in the database" << std::endl;

				// -- RES_GEOM

				sql = "CREATE TABLE IF NOT EXISTS RES_GEOM("  \
				"ID SERIAL UNIQUE PRIMARY KEY				NOT NULL," \
				"ID_MEAS INT REFERENCES MEASUREMENTS(ID)	NOT NULL," \
				"UOM			  TEXT								," \
				"RESULT	      GEOMETRY					NOT NULL" \
				");";
      
				/* Execute SQL query */
				pqxx::work Wd(*C);
				Wd.exec( sql );
				Wd.commit();
				std::cout << "Table RES_GEOM in the database" << std::endl;

				//C.disconnect ();
			}
			catch (const std::exception &e)
			{
				std::cout << e.what() << std::endl;
				return false;
			}
			return true;
		}

		std::string SosReader::toSqlTimeStamp(time_t time, pqxx::connection * C)
		{

			std::string output;

				if (!C->is_open()) {
					std::cout << "toSqlTimeStamp: Database not opened" << std::endl;
					return output;
				}

				try
				{
				std::stringstream sqlSs;
	
				sqlSs << "SELECT to_timestamp(" << time << ")::timestamp;";

				/* Create a non-transactional object. */
				pqxx::nontransaction N(*C);
      
				/* Execute SQL query */
				pqxx::result R( N.exec( sqlSs ));

				output = R.begin()[0].as<std::string>();

				}
				catch (const std::exception &e)
				{
				std::cout << "toSqlTimeStamp: " << e.what() << std::endl;
				return output;
				}
				return output;
		}

		bool SosReader::isResultSaved(long long rId, const std::string & rUom, const std::string & rTableName, pqxx::connection * C)
		{
			if (!C->is_open()) {
					std::cout << "isResultSaved: Database not opened" << std::endl;
					return false;
				}

				try
				{

				std::stringstream sqlSs;
				sqlSs << "SELECT EXISTS(SELECT 1 FROM " << rTableName << " WHERE ";
				sqlSs << "ID_MEAS = " << rId << " AND ";
				sqlSs << "UOM = '" << rUom << "'";
				sqlSs << ") AS \"EXISTS\";";

				/* Create a non-transactional object. */
				pqxx::nontransaction N(*C);
      
				/* Execute SQL query */
				pqxx::result R( N.exec( sqlSs ));
      
				return R.begin()[0].as<bool>();
				}
				catch (const std::exception &e)
				{
				std::cout << "isResultSaved: " << e.what() << std::endl;
				return false;
				}
		}

		template <>
		std::string SosReader::getResultTableName<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>()
		{
			return "RES_REAL";
		}

		template <>
		std::string SosReader::getResultTableName<MobileNoise::SOS::OM_Measurement<MobileNoise::SOS::Point, MobileNoise::SOS::Point>>()
		{
			return "RES_GEOM";
		}

		template <>
		std::string SosReader::getResultTableName<MobileNoise::SOS::OM_Measurement<int, MobileNoise::SOS::Point>>()
		{
			return "RES_INT";
		}

		template <>
		std::string SosReader::getResultTableName<MobileNoise::SOS::OM_Measurement<std::string, MobileNoise::SOS::Point>>()
		{
			return "RES_TEXT";
		}

		std::stringstream SosReader::getOneObservationJson()
		{
			std::stringstream observation;

			if (!cs.hasData())
				return observation;

			char c;
	
			// searching for opening observation "{"
			while (cs.get(c))
			{
				if ('{' == c)
				{
					observation << c;
					break;
				}
				else if ('}' == c)
				{
					//there is no necessity to search for rest of file
					return observation;
				}
			}
			size_t deep = 1;

			// searching for closing observation "}"
			while ((deep > 0) && (cs.get(c)))   
			{
				if ('{' == c)
				{
					deep++;
				}
				else if ('}' == c)
				{
					deep--; 
				}
				observation << c;
				//std::cout << c;
			}

			if (0 == deep)
			{
				//std::cout << observation.str() << std::endl;
				return observation;
			}
			return std::stringstream();
		}

		

//const char data[]="{ \"request\": \"GetObservation\",\"service\": \"SOS\",\"version\": \"2.0.0\",\"procedure\": \"urn.meters.sound.mobile.phones.MACNotFound\",\"offering\": \"http://mobile-noise.ddns.net/offering/urn:muni:def:network:mobile:noise2015/observations\",\"observedProperty\": \"http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise\",\"featureOfInterest\": \"urn:muni:def:world:environment:noise\"}";
 
		

 
		size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
		{
		  struct SosReader::WriteThis *pooh = (struct SosReader::WriteThis *)userp;
 
		  if(size*nmemb < 1)
			return 0;
 
		  if(pooh->sizeleft) {
			*(char *)ptr = pooh->readptr[0]; /* copy one single byte */ 
			pooh->readptr++;                 /* advance pointer */ 
			pooh->sizeleft--;                /* less data left */ 
			return 1;                        /* we return 1 byte at a time! */ 
		  }
 
		  return 0;                          /* no more data left to deliver */ 
		}

//static std::string URL = "http://localhost:8080/52n-sos-webapp/service";

		int writer(char *data, size_t size, size_t nmemb, MobileNoise::SOS::CachingStream *cstream)
		{
			std::stringstream inputStream;//(data, size * nmemb);

			for (size_t i = 0; i < (size * nmemb); i++)
			{
				inputStream << *data;
				data++;
			}

			cstream->insertString(inputStream.str());

			return size * nmemb;  
		} 
 
		int SosReader::getObsFromServer(const std::string & rUrl, const std::string & rReqSosJson)
		{

		  CURL *curl;
		  CURLcode res;
 
		  struct WriteThis pooh;
 
		  pooh.readptr = rReqSosJson.c_str();
		  pooh.sizeleft = (long long)strlen(rReqSosJson.c_str());
 
		  /* In windows, this will init the winsock stuff */ 
		  res = curl_global_init(CURL_GLOBAL_DEFAULT);
		  /* Check for errors */ 
		  if(res != CURLE_OK) {
			fprintf(stderr, "curl_global_init() failed: %s\n",
					curl_easy_strerror(res));
			return 1;
		  }

			struct curl_slist *headers=NULL; // init to NULL is important 
			std::ostringstream oss;
			headers = curl_slist_append( headers, "Accept: application/json");  
			headers = curl_slist_append( headers, "Content-Type: application/json");
			headers = curl_slist_append( headers, "charsets: utf-8"); 
 
		  /* get a curl handle */ 
		  curl = curl_easy_init();
		  if(curl) {

			  cs.beginStreaming();

			//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

			/* First set the URL that is about to receive our POST. */ 
			curl_easy_setopt(curl, CURLOPT_URL, rUrl.c_str());
 
			/* Now specify we want to POST data */ 
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
 
			/* we want to use our own read function */ 
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,writer);
 
			/* pointer to pass to our read function */ 
			curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);

			/* we pass our 'chunk' struct to the callback function */ 
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cs);


 
			/* get verbose debug output please */ 
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
			/*
			  If you use POST to a HTTP 1.1 server, you can send data without knowing
			  the size before starting the POST if you use chunked encoding. You
			  enable this by adding a header like "Transfer-Encoding: chunked" with
			  CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must
			  specify the size in the request.
			*/ 
		#ifdef USE_CHUNKED
			{
			  //struct curl_slist *chunk = NULL;
 
			  headers = curl_slist_append(headers, "Transfer-Encoding: chunked");
			  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			  /* use curl_slist_free_all() after the *perform() call to free this
				 list again */ 
			}
		#else
			/* Set the expected POST size. If you want to POST large amounts of data,
			   consider CURLOPT_POSTFIELDSIZE_LARGE */ 
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pooh.sizeleft);
		#endif
 
		#ifdef DISABLE_EXPECT
			/*
			  Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue"
			  header.  You can disable this header with CURLOPT_HTTPHEADER as usual.
			  NOTE: if you want chunked transfer too, you need to combine these two
			  since you can only set one list of headers with CURLOPT_HTTPHEADER. */ 
 
			/* A less good option would be to enforce HTTP 1.0, but that might also
			   have other implications. */ 
			{
			  struct curl_slist *chunk = NULL;
 
			  chunk = curl_slist_append(chunk, "Expect:");
			  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
			  /* use curl_slist_free_all() after the *perform() call to free this
				 list again */ 
			}
		#endif

			//isConnected = true;
 
			/* Perform the request, res will get the return code */ 
			res = curl_easy_perform(curl);

			cs.endStreaming();
			isConnected = false;

			/* Check for errors */ 
			if(res != CURLE_OK)
			  fprintf(stderr, "curl_easy_perform() failed: %s\n",
					  curl_easy_strerror(res));
 
			/* always cleanup */ 
			curl_easy_cleanup(curl);

		  }
		  curl_slist_free_all(headers);
		  curl_global_cleanup();
		  return 0;
		}

		int SosReader::pullAllData(const std::string & sqlConnectionString, const std::string & sosServiceUrl, const std::string & requestJsonFilePath, const std::string & obsIdent, const std::string & resultObsProp, const std::string & resultUom)
		{
			int status = pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>(sqlConnectionString, sosServiceUrl, "GetObservationsNoise.json", obsIdent);
			status = pullData<MobileNoise::SOS::OM_Measurement<MobileNoise::SOS::Point, MobileNoise::SOS::Point>>(sqlConnectionString, sosServiceUrl, "GetObservationsLocation.json", obsIdent, resultObsProp, "4326");
			status = pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>(sqlConnectionString, sosServiceUrl, "GetObservationsStreetSide.json", obsIdent, resultObsProp, "degrees_turn");
			status = pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>(sqlConnectionString, sosServiceUrl, "GetObservationsTurn.json", obsIdent, resultObsProp, "degrees_street_side");
			return status;
		}

	} // namespace SOS
} // namespace MobileNoise


//int main(void)
//{
//	MobileNoise::SOS::SosReader r;
//
//	int status = r.pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432", "http://localhost:8080/52n-sos-webapp/service", "GetObservationsNoise.json", "N/A");
//	status = r.pullData<MobileNoise::SOS::OM_Measurement<MobileNoise::SOS::Point, MobileNoise::SOS::Point>>("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432", "http://localhost:8080/52n-sos-webapp/service", "GetObservationsLocation.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "4326");
//	status = r.pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432", "http://localhost:8080/52n-sos-webapp/service", "GetObservationsStreetSide.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_turn");
//	status = r.pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432", "http://localhost:8080/52n-sos-webapp/service", "GetObservationsTurn.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_street_side");
//	//int status = r.readSosData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("GetObservationsNoiseResponse.json", "N/A");
//	//status = r.readSosData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("GetObservationsTurnResponse.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_turn");
//	//status = r.readSosData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("GetObservationsStreetSideResponse.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_street_side");
//	//status = r.readSosData<MobileNoise::SOS::OM_Measurement<MobileNoise::SOS::Point, MobileNoise::SOS::Point>>("GetObservationsLocationResponse.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_loc");
//
//	return status;
//}

