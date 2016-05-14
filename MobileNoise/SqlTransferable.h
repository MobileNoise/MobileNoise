#include <string>

namespace MobileNoise
{
	struct SqlTransferable
	{
		SqlTransferable(){}
		virtual ~SqlTransferable(){}

		virtual std::string toSqlString() = 0;

	};

	template< typename T >
	struct TypedSqlTransferable : public SqlTransferable
	{
		virtual std::string toSqlString() override;
		virtual bool toSql(T * rReadedObj) = 0;
		virtual bool fromSql(T * rReadedObj) = 0;
	};
} // namespace MobileNoise