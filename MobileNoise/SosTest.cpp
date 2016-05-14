//#pragma once
//
//#include "Sos.h"
//
//#include "json/json.h"
//#include <pqxx/pqxx>
//
//#include <iostream>
//#include <map>
//#include <fstream>
//#include <vector>
//
//static const std::string OBS_STR = "observations\"";
//static const std::string OBS_FILENAME = "GetObservationsResponse.json";
//
//bool createMeasTablesIfNotExist(pqxx::connection * C)
//{
//	try
//	{
//      //pqxx::connection C("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432");
//      if (C->is_open()) 
//	  {
//         std::cout << "Opened database successfully: " << C->dbname() << std::endl;
//      } 
//	  else 
//	  {
//         std::cout << "Can't open database" << std::endl;
//         return false;
//      }
//
//	  char * sql;
//
//	  // -- MEASUREMENTS
//
//	   /* Create SQL statement */
//      sql = "CREATE TABLE IF NOT EXISTS MEASUREMENTS("  \
//      "ID SERIAL UNIQUE PRIMARY KEY NOT NULL," \
//      "IDENT		  TEXT		NOT NULL," \
//	  "IDENT_CS       TEXT				," \
//	  "PROCEDURE      TEXT		NOT NULL," \
//	  "OFFERING		  TEXT				," \
//	  "OBS_PROP		  TEXT				," \
//	  "FOI_IDENT	  TEXT		NOT NULL," \
//	  "FOI_IDENT_CS	  TEXT				," \
//	  "FOI_NAME		  TEXT				," \
//	  "FOI_NAME_CS	  TEXT				," \
//	  "FOI_SAMP_FE	  TEXT		NOT NULL," \
//	  "FOI_GEOMETRY	  GEOMETRY			," \
//	  "PHENOM_TIME	  TIMESTAMP NOT NULL," \
//	  "RESULT_TIME	  TIMESTAMP NOT NULL" \
//	  ");";
//
//	  
//      /* Create a transactional object. */
//      pqxx::work W(*C);
//      
//      /* Execute SQL query */
//      W.exec( sql );
//      W.commit();
//      std::cout << "Table MEASUREMENTS in the database" << std::endl;
//
//	  // -- RES_REAL
//
//	  sql = "CREATE TABLE IF NOT EXISTS RES_REAL("  \
//      "ID SERIAL UNIQUE PRIMARY KEY				NOT NULL," \
//	  "ID_MEAS INT REFERENCES MEASUREMENTS(ID)	NOT NULL," \
//      "UOM			  TEXT								," \
//	  "RESULT	      REAL						NOT NULL" \
//	  ");";
//      
//      /* Execute SQL query */
//	  pqxx::work Wa(*C);
//      Wa.exec( sql );
//      Wa.commit();
//      std::cout << "Table RES_REAL in the database" << std::endl;
//
//	  // -- RES_INT
//
//	  sql = "CREATE TABLE IF NOT EXISTS RES_INT("  \
//      "ID SERIAL UNIQUE PRIMARY KEY				NOT NULL," \
//	  "ID_MEAS INT REFERENCES MEASUREMENTS(ID)	NOT NULL," \
//      "UOM			  TEXT								," \
//	  "RESULT	      INT						NOT NULL" \
//	  ");";
//      
//      /* Execute SQL query */
//	  pqxx::work Wb(*C);
//      Wb.exec( sql );
//      Wb.commit();
//      std::cout << "Table RES_INT in the database" << std::endl;
//
//	  // -- RES_TEXT
//
//	  sql = "CREATE TABLE IF NOT EXISTS RES_TEXT("  \
//      "ID SERIAL UNIQUE PRIMARY KEY				NOT NULL," \
//	  "ID_MEAS INT REFERENCES MEASUREMENTS(ID)	NOT NULL," \
//      "UOM			  TEXT								," \
//	  "RESULT	      TEXT						NOT NULL" \
//	  ");";
//      
//      /* Execute SQL query */
//	  pqxx::work Wc(*C);
//      Wc.exec( sql );
//      Wc.commit();
//      std::cout << "Table RES_TEXT in the database" << std::endl;
//
//	  // -- RES_GEOM
//
//	  sql = "CREATE TABLE IF NOT EXISTS RES_GEOM("  \
//      "ID SERIAL UNIQUE PRIMARY KEY				NOT NULL," \
//	  "ID_MEAS INT REFERENCES MEASUREMENTS(ID)	NOT NULL," \
//      "UOM			  TEXT								," \
//	  "RESULT	      GEOMETRY					NOT NULL" \
//	  ");";
//      
//      /* Execute SQL query */
//	  pqxx::work Wd(*C);
//      Wd.exec( sql );
//      Wd.commit();
//      std::cout << "Table RES_GEOM in the database" << std::endl;
//
//      //C.disconnect ();
//    }
//	catch (const std::exception &e)
//	{
//      std::cout << e.what() << std::endl;
//      return false;
//    }
//	return true;
//}
//
//std::string toSqlTimeStamp(time_t time, pqxx::connection * C)
//{
//
//	std::string output;
//
//	 if (!C->is_open()) {
//         std::cout << "toSqlTimeStamp: Database not opened" << std::endl;
//		 return output;
//     }
//
//	 try
//	 {
//		std::stringstream sqlSs;
//	
//		sqlSs << "SELECT to_timestamp(" << time << ")::timestamp;";
//
//		/* Create a non-transactional object. */
//		pqxx::nontransaction N(*C);
//      
//		/* Execute SQL query */
//		pqxx::result R( N.exec( sqlSs ));
//
//		output = R.begin()[0].as<std::string>();
//
//	 }
//	 catch (const std::exception &e)
//	 {
//		std::cout << "toSqlTimeStamp: " << e.what() << std::endl;
//		return output;
//	 }
//	 return output;
//}
//
//bool isResultSaved(long long rId, const std::string & rUom, const std::string & rTableName, pqxx::connection * C)
//{
//	if (!C->is_open()) {
//         std::cout << "isResultSaved: Database not opened" << std::endl;
//         return false;
//      }
//
//	  try
//	  {
//
//	  std::stringstream sqlSs;
//	  sqlSs << "SELECT EXISTS(SELECT 1 FROM " << rTableName << " WHERE ";
//	  sqlSs << "ID_MEAS = " << rId << " AND ";
//	  sqlSs << "UOM = '" << rUom << "'";
//	  sqlSs << ") AS \"EXISTS\";";
//
//      /* Create a non-transactional object. */
//      pqxx::nontransaction N(*C);
//      
//      /* Execute SQL query */
//      pqxx::result R( N.exec( sqlSs ));
//      
//      return R.begin()[0].as<bool>();
//	 }
//	 catch (const std::exception &e)
//	 {
//		std::cout << "isResultSaved: " << e.what() << std::endl;
//		return false;
//	 }
//}
//
//template <typename Meas>
//std::string getResultTableName();
//
//template <>
//std::string getResultTableName<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>()
//{
//	return "RES_REAL";
//}
//
//template <>
//std::string getResultTableName<MobileNoise::SOS::OM_Measurement<MobileNoise::SOS::Point, MobileNoise::SOS::Point>>()
//{
//	return "RES_GEOM";
//}
//
//template <>
//std::string getResultTableName<MobileNoise::SOS::OM_Measurement<int, MobileNoise::SOS::Point>>()
//{
//	return "RES_INT";
//}
//
//template <>
//std::string getResultTableName<MobileNoise::SOS::OM_Measurement<std::string, MobileNoise::SOS::Point>>()
//{
//	return "RES_TEXT";
//}
//
//
//template <typename Meas>
//bool isMeasSaved( Meas * meas, pqxx::connection * C)
//{
//
//	  if (!C->is_open()) {
//         std::cout << "isMeasSaved: Database not opened" << std::endl;
//         return false;
//      }
//
//	  try
//	  {
//
//		  std::string phenomenonTime = toSqlTimeStamp(meas->phenomenonTime.time, C);
//		  std::string resultTime = toSqlTimeStamp(meas->resultTime.time, C);
//
//		  std::stringstream sqlSs;
//		  sqlSs << "SELECT EXISTS(SELECT 1 FROM MEASUREMENTS WHERE ";
//		  sqlSs << "IDENT = \'" << meas->identifier.value << "\' AND ";
//		  sqlSs << "PROCEDURE = \'" << meas->procedure << "\' AND ";
//		  sqlSs << "OBS_PROP = \'" << meas->observableProperty << "\' AND ";
//		  sqlSs << "PHENOM_TIME = \'" << phenomenonTime << "\' AND ";
//		  sqlSs << "RESULT_TIME = \'" << resultTime  << "\'";
//		  sqlSs << ") AS \"EXISTS\";";
//
//		  /* Create a non-transactional object. */
//		  pqxx::nontransaction N(*C);
//      
//		  /* Execute SQL query */
//		  pqxx::result R( N.exec( sqlSs ));
//      
//		  return R.begin()[0].as<bool>();
//	 }
//	 catch (const std::exception &e)
//	 {
//		std::cout << "isMeasSaved: " << e.what() << std::endl;
//		return false;
//	 }
//
//}
//
//template <typename Meas>
//long long getMeasId( Meas * meas, pqxx::connection * C)
//{
//	if (!C->is_open()) 
//	{
//		std::cout << "getMeasId: Database not opened" << std::endl;
//		return LLONG_MIN;
//	}
//
//	try
//	{
//	  std::string phenomenonTime = toSqlTimeStamp(meas->phenomenonTime.time, C);
//	  std::string resultTime = toSqlTimeStamp(meas->resultTime.time, C);
//
//	  std::stringstream sqlSs;
//	  sqlSs << "SELECT ID FROM MEASUREMENTS WHERE ";
//	  sqlSs << "IDENT = \'" << meas->identifier.value << "\' AND ";
//	  sqlSs << "PROCEDURE = \'" << meas->procedure << "\' AND ";
//	  sqlSs << "OBS_PROP = \'" << meas->observableProperty << "\' AND ";
//	  sqlSs << "PHENOM_TIME = \'" << phenomenonTime << "\' AND ";
//	  sqlSs << "RESULT_TIME = \'" << resultTime << "\'";
//	  sqlSs << ";";
//
//      /* Create a non-transactional object. */
//      pqxx::nontransaction N(*C);
//      
//      /* Execute SQL query */
//      pqxx::result R( N.exec( sqlSs ));
//
//	  if (R.begin() == R.end())
//	  {
//		  std::cout << "getMeasId: Assigned Measurement not found in DB" << std::endl;
//		  return LLONG_MIN;
//	  }
//      
//      return R.begin()[0].as<long long>();
//	}
//	catch (const std::exception &e)
//	{
//		std::cout << "getMeasId: " << e.what() << std::endl;
//		return false;
//	}
//}
//
//template <typename Meas>
//bool saveResult( Meas * meas, pqxx::connection * C)//;
//
////template <>
////bool saveResult<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>( MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point> * meas, pqxx::connection * C)
//{
//	if (!C->is_open()) 
//	{
//		std::cout << "saveResult: Database not opened" << std::endl;
//		return false;
//	}
//
//	try
//	{
//	  long long measId = getMeasId<Meas>(meas, C);
//
//	  if (LLONG_MIN == measId)
//	  {
//		  return false;
//	  }
//
//	  if (isResultSaved(measId, meas->result.uom, getResultTableName<Meas>(), C))
//	  {
//		  std::stringstream sqlSs;
//		  /* Create a transactional object. */
//		  pqxx::work W(*C);
//		  /* Create  SQL UPDATE statement */
//		  sqlSs << "UPDATE " << getResultTableName<Meas>() << " set RESULT = " << meas->result.valueToString() << " ,";
//		  sqlSs << " UOM = \'" << meas->result.uom << "\'";
//		  sqlSs << " WHERE ID_MEAS = " << measId << ";";
//		  /* Execute SQL query */
//		  W.exec( sqlSs );
//		  W.commit();
//		  std::cout << "Result " << meas->result.valueToString() << " updated" << std::endl;
//	  }
//	  else
//	  {
//		  std::stringstream sqlSs;
//		   /* Create SQL statement */
//		  sqlSs << "INSERT INTO " << getResultTableName<Meas>() << "(ID_MEAS,UOM,RESULT) ";  
//		  sqlSs << "VALUES (" << measId << ",\'" << meas->result.uom << "\'," << meas->result.toSqlString() << "); ";
//
//		  /* Create a transactional object. */
//		  pqxx::work W(*C);
//      
//		  /* Execute SQL query */
//		  W.exec( sqlSs );
//		  W.commit();
//		  std::cout << "Result " << meas->result.valueToString() << " saved" << std::endl;
//
//	  }
//      
//      return true;
//	}
//	catch (const std::exception &e)
//	{
//		std::cout << "saveResult: " << e.what() << std::endl;
//		return false;
//	}
//}
//
//template <typename Meas>
//bool saveMeas( Meas * meas, pqxx::connection * C)
//{
//	if (!C->is_open()) {
//         std::cout << "saveMeas: Database not opened" << std::endl;
//         return false;
//      }
//
//	  try
//	  {
//		std::string phenomenonTime = toSqlTimeStamp(meas->phenomenonTime.time, C);
//		std::string resultTime = toSqlTimeStamp(meas->resultTime.time, C);
//		
//		char * sql;
//		sql = "INSERT INTO MEASUREMENTS" \
//				"(" \
//				"IDENT," \
//				"IDENT_CS," \
//				"PROCEDURE," \
//				"OFFERING," \
//				"OBS_PROP," \
//				"FOI_IDENT," \
//				"FOI_IDENT_CS," \
//				"FOI_NAME," \
//				"FOI_NAME_CS," \
//				"FOI_SAMP_FE," \
//				"FOI_GEOMETRY," \
//				"PHENOM_TIME," \
//				"RESULT_TIME" \
//				") VALUES (";
//
//		std::stringstream sqlSs;
//		sqlSs << sql;
//		sqlSs << "\'" << meas->identifier.value << "\',";
//		sqlSs << "\'" << meas->identifier.codespace << "\',";
//		sqlSs << "\'" << meas->procedure << "\',";
//		sqlSs << "\'" << meas->offering << "\',";
//		sqlSs << "\'" << meas->observableProperty << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.identifier.value << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.identifier.codespace << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.name.value << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.identifier.codespace << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.sampledFeature << "\',";
//		sqlSs << "ST_GeomFromText(\'" << meas->featureOfInterest.geometry.toSqlString() << "\', 4326),";
//		sqlSs << "\'" << phenomenonTime << "\',";
//		sqlSs << "\'" << resultTime << "\');";
//
//		 /* Create a transactional object. */
//      pqxx::work W(*C);
//      
//      /* Execute SQL query */
//	  W.exec( sqlSs );
//      W.commit();
//
//	  return saveResult<Meas>(meas, C);
//
//	 }
//	 catch (const std::exception &e)
//	 {
//		std::cout << "saveMeas: " << e.what() << std::endl;
//		return false;
//	 }
//
//}
//
//template <typename Meas>
//bool updateMeas( Meas * meas, pqxx::connection * C)
//{
//	if (!C->is_open()) {
//         std::cout << "updateMeas: Database not opened" << std::endl;
//         return false;
//    }
//
//	try
//	{
//	    long long measId = getMeasId<Meas>(meas, C);
//
//	    if (LLONG_MIN == measId)
//		{
//			return false;
//		}
//
//		std::string phenomenonTime = toSqlTimeStamp(meas->phenomenonTime.time, C);
//		std::string resultTime = toSqlTimeStamp(meas->resultTime.time, C);
//		
//		char * sql;
//		sql = "UPDATE MEASUREMENTS set " \
//				"(" \
//				"IDENT," \
//				"IDENT_CS," \
//				"PROCEDURE," \
//				"OFFERING," \
//				"OBS_PROP," \
//				"FOI_IDENT," \
//				"FOI_IDENT_CS," \
//				"FOI_NAME," \
//				"FOI_NAME_CS," \
//				"FOI_SAMP_FE," \
//				"FOI_GEOMETRY," \
//				"PHENOM_TIME," \
//				"RESULT_TIME" \
//				") = (";
//
//		std::stringstream sqlSs;
//		sqlSs << sql;
//		sqlSs << "\'" << meas->identifier.value << "\',";
//		sqlSs << "\'" << meas->identifier.codespace << "\',";
//		sqlSs << "\'" << meas->procedure << "\',";
//		sqlSs << "\'" << meas->offering << "\',";
//		sqlSs << "\'" << meas->observableProperty << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.identifier.value << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.identifier.codespace << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.name.value << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.identifier.codespace << "\',";
//		sqlSs << "\'" << meas->featureOfInterest.sampledFeature << "\',";
//		sqlSs << "ST_GeomFromText(\'" << meas->featureOfInterest.geometry.toSqlString() << "\', 4326),";
//		sqlSs << "\'" << phenomenonTime << "\',";
//		sqlSs << "\'" << resultTime << "\')";
//
//		sqlSs << " WHERE " << "ID = " << measId << ";";
//
//			/* Create a transactional object. */
//		pqxx::work W(*C);
//      
//		/* Execute SQL query */
//		W.exec( sqlSs );
//		W.commit();
//
//		return saveResult<Meas>(meas, C);
//
//	 }
//	 catch (const std::exception &e)
//	 {
//		std::cout << "updateMeas: " << e.what() << std::endl;
//		return false;
//	 }
//
//}
//
//template <typename Meas>
//bool saveObsIntoDB(Json::Value & rObsJson, pqxx::connection * C, const std::string & obsIdent = "",const std::string & resultObsProp = "", const std::string & resultUom = "")
//{
//	try
//	{
//      //pqxx::connection C("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432");
//      if (!C->is_open()) 
//	  {
//         std::cout << "Can't open database" << std::endl;
//         return false;
//      }
//
//
//	  Meas meas;
//	  bool success = meas.fromJson(rObsJson);
//
//	  if (!success)
//	  {
//		  std::cout << "Unable to parse json" << std::endl;
//		  return false;
//	  }
//
//	  // changing observation id according to parameter 
//	  if (!obsIdent.empty())
//	  {
//		  meas.identifier.value = obsIdent;
//	  }
//
//	  // changing observable property according to parameter (not converting now, just changing label)
//	  if (!resultObsProp.empty())
//	  {
//		  meas.observableProperty = resultObsProp;
//	  }
//
//	  // changing uom according to parameter (not converting now, just changing label)
//	  if (!resultUom.empty())
//	  {
//		  meas.result.uom = resultUom;
//	  }
//
//	  //test if value is saved
//	  if (isMeasSaved<Meas>(&meas, C))
//	  {
//		  success = updateMeas<Meas>(&meas, C);
//	  }
//	  else
//	  {
//		  success = saveMeas<Meas>(&meas, C);
//	  }
//
//	  //char * sql;
//
//	  // /* Create SQL statement */
//   //   sql = "SELECT exists(select 1 from contact where id=12) AS \"exists\";";
//
//   //    /* Create a non-transactional object. */
//   //   pqxx::nontransaction N(*C);
//   //   
//   //   /* Execute SQL query */
//   //   pqxx::result R( N.exec( sql ));
//   //   
//   //   /* List down all the records */
//   //   for (pqxx::result::const_iterator r = R.begin(); r != R.end(); ++r) {
//   //      cout << "ID = " << c[0].as<int>() << endl;
//   //      cout << "Name = " << c[1].as<string>() << endl;
//   //      cout << "Age = " << c[2].as<int>() << endl;
//   //      cout << "Address = " << c[3].as<string>() << endl;
//   //      cout << "Salary = " << c[4].as<float>() << endl;
//   //   }
//   //   cout << "Operation done successfully" << endl;
//
//
//      //C.disconnect ();
//    }
//	catch (const std::exception &e)
//	{
//      std::cout << e.what() << std::endl;
//      return false;
//    }
//	return true;
//}
//
//std::stringstream getOneObservationJson(std::ifstream * rOpenFile)
//{
//	std::stringstream observation;
//
//	if ((nullptr == rOpenFile) || (!rOpenFile->is_open()))
//		return observation;
//
//	char c;
//	
//	// searching for opening observation "{"
//	while (rOpenFile->get(c))
//	{
//		if ('{' == c)
//		{
//			observation << c;
//			break;
//		}
//		else if ('}' == c)
//		{
//			//there is no necessity to search for rest of file
//			return observation;
//		}
//	}
//	size_t deep = 1;
//
//	// searching for closing observation "}"
//	while ((deep > 0) && (rOpenFile->get(c)))   
//	{
//		if ('{' == c)
//		{
//			deep++;
//		}
//		else if ('}' == c)
//		{
//			deep--; 
//		}
//		observation << c;
//		//std::cout << c;
//	}
//
//	if (0 == deep)
//	{
//		return observation;
//	}
//	return std::stringstream();
//}
//
//template <typename Meas>
//int readSosData(const std::string & filename, const std::string & obsIdent = "", const std::string & resultObsProp = "", const std::string & resultUom = "")
//{
//
//  std::ifstream sosOutput(filename);
//  if (sosOutput.is_open())
//  {
//	  int err = 2;
//	  char c;
//	  sosOutput.seekg(0, sosOutput.beg);
//
//	  // searching for "observations"
//	  while (sosOutput.get(c))    
//	  {
//		  if ('\"' == c)
//		  {
//			  bool found = false;
//			  size_t index = 0;
//			  
//			  while (sosOutput.get(c))
//			  {
//				  if (c == OBS_STR[index])
//				  {
//					  index++;
//					  continue;
//				  }
//				  else if (index == OBS_STR.size())
//				  {
//					  found = true;
//					  break;
//				  }
//				  found = false;
//				  break;
//			  }
//
//			  if (found)
//			  {
//				  err = 0;
//				  break;
//			  }
//			 
//		  }
//	  }
//
//	  if (err != 0)
//	  {
//		  sosOutput.close();
//		  std::cout << "Observations in file " << filename << " not found." <<std::endl;
//		  return err;
//	  }
//
//	  bool obsNotFound = true;
//
//	  // searching for first "["
//	  while (sosOutput.get(c))    
//	  {
//		  if ('[' == c)
//		  {
//			  obsNotFound = false;
//			  break;
//		  }
//	  }
//
//	  if (obsNotFound)
//	  {
//		  sosOutput.close();
//		  std::cout << "Observations in file " << filename << " not found." <<std::endl;
//		  return err;
//	  }
//
//	  try{
//		  pqxx::connection C("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432");
//
//		  bool run = createMeasTablesIfNotExist(&C);
//
//		  std::vector<Json::Value> observations;
//		  while (run)
//		  {
//			  std::stringstream obsSs = getOneObservationJson(&sosOutput);
//
//			  obsSs.seekg(0, std::ios::end);
//			  std::streamoff size = obsSs.tellg();
//			  if (0 == size)
//			  {
//				  break;
//			  }
//			  obsSs.seekg(0, std::ios::beg);
//			  Json::Value obsJs;
//			  obsSs >> obsJs;
//			  //MobileNoise::SOS::OM_Measurement<double,MobileNoise::SOS::Point> meas;
//			  //observations.push_back(obsJs);
//
//			  saveObsIntoDB<Meas>(obsJs, &C, obsIdent, resultObsProp, resultUom);
//
//			  //std::cout << obsJs << std::endl << "----------------------------------" << std::endl;
//		  }
//		  C.disconnect ();
//	  }
//	  catch(const std::exception &e)
//	  {
//		  std::cout << e.what() << std::endl;
//		  return 3;
//	  }
//	  sosOutput.close();
//	  return 0;
//  }
//  std::cout << "File " << filename << " not found." <<std::endl;
//  return 1;
//
//}
//
//
//int main(void)
//{
//	
//	int status = readSosData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("GetObservationsNoiseResponse.json", "N/A");
//	status = readSosData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("GetObservationsTurnResponse.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_turn");
//	status = readSosData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("GetObservationsStreetSideResponse.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_street_side");
//	status = readSosData<MobileNoise::SOS::OM_Measurement<MobileNoise::SOS::Point, MobileNoise::SOS::Point>>("GetObservationsLocationResponse.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_loc");
//
//	return status;
//}