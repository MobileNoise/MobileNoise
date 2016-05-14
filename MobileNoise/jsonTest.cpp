//#pragma once
//
//#include "Sos.h"
//
//#include <iostream>
//#include "json/json.h"
//#include <map>
//
//
//int main(void)
//{
//	std::stringstream json;
//	json << "{" ;
//    json << "\"encoding\" : \"UTF-8\", ";
//    json << "\"plug-ins\" : [ ";
//    json << "    \"python\", ";
//    json << "    \"c++\", ";
//    json << "    \"ruby\" ";
//    json << "    ], ";
//    json << "\"indent\" : { \"length\" : 3, \"use_space\": true }";
//	json << "}";
//
//	std::vector<std::string> plugIns;
//
//
//	Json::Value root;   // 'root' will contain the root value after parsing.
//	//std::cin >> root;
//	json >> root;
//	// You can also read into a particular sub-value.
//	//std::cin >> root["subtree"];
//	// Get the value of the member of root named 'encoding',
//	// and return 'UTF-8' if there is no such member.
//	std::string encoding = root.get("encoding", "UTF-8" ).asString();
//	// Get the value of the member of root named 'plug-ins'; return a 'null' value if
//	// there is no such member.
//	const Json::Value plugins = root["plug-ins"];
//	// Iterate over the sequence elements.
//	for ( unsigned index = 0; index < plugins.size(); ++index )
//		plugIns.push_back( plugins[index].asString() );
//   
//	// Try other datatypes. Some are auto-convertible to others.
//	int indentLength = root["indent"].get("length", 3).asInt();
//	bool indentUseSpace = root["indent"].get("use_space", true).asBool();
//	// Since Json::Value has an implicit constructor for all value types, it is not
//	// necessary to explicitly construct the Json::Value object.
//	root["encoding"] = "bla";
//	root["indent"]["length"] = 2;
//	root["indent"]["use_space"] = false;
//	// If you like the defaults, you can insert directly into a stream.
//	std::cout << root;
//	// Of course, you can write to `std::ostringstream` if you prefer.
//	// If desired, remember to add a linefeed and flush.
//	std::cout << std::endl;
//	// For convenience, use `writeString()` with a specialized builder.
//	Json::StreamWriterBuilder wbuilder;
//	wbuilder["indentation"] = "\t";
//	std::string document = Json::writeString(wbuilder, root);
//	// Here, using a specialized Builder, we discard comments and
//	// record errors as we parse.
//	Json::CharReaderBuilder rbuilder;
//	rbuilder["collectComments"] = false;
//	std::string errs;
//	bool ok = Json::parseFromStream(rbuilder, json, &root, &errs);
//	//Json::CharReaderBuilder rbuilder;
//	//cfg >> rbuilder.settings_;
//	//std::unique_ptr<Json::CharReader> const reader(rbuilder.newCharReader());
//	//reader->parse(start, stop, &value1, &errs);
//	//// ...
//	//reader->parse(start, stop, &value2, &errs);
//	// etc.
//	return 0;
//}
