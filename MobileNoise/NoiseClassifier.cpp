# include "NoiseClassifier.h"
# include "NoiseReaderWriter.h"

namespace MobileNoise
{
	template<>
	bool NoiseClassifier<pqxx::connection>::determineAffectedArea(const std::string & rDbConnStr, const std::string & rTableName, const std::string & rAffectedViewName, std::vector<std::string> & rCriteria)
	{
		NoiseReaderWriter<pqxx::connection> nrw = NoiseReaderWriter<pqxx::connection>(rDbConnStr);
		if (!nrw.initialize())
		{
			return false;
		}
		std::vector<std::string> tableNames;
		tableNames.push_back(rTableName);
		std::vector<std::string> visibleCols;

		return nrw.createView(tableNames, rAffectedViewName, rCriteria, visibleCols);
	}
} // namespace MobileNoise