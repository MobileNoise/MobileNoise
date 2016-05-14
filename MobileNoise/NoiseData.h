# include "Sos.h"

namespace MobileNoise
{
	class NoisePoint : SOS::Point
	{
	public:
		enum Turn {Turn_unknown, Turn_left, Turn_center, Turn_right} mTurn;
		enum Side {Side_unknown, Side_left, Side_center, Side_right} mSide;
		time_t mMeasTime;
		double Laeq;
		long long serieId;
		double uncert;
		std::string deviceId;

		NoisePoint()
			: SOS::Point()
			, mTurn(Turn_unknown)
			, mSide(Side_unknown)
			, mMeasTime(0)
			, Laeq(0)
			, uncert(std::numeric_limits<double>::infinity())
			, serieId(LLONG_MIN)
		{
		}

		NoisePoint(double rLat, double rLon, const std::string & rSrid, double rLaeq, double rUncert, time_t rMeasTime, Turn rTurn, Side rSide, const std::string & rDeviceId, long long rSerieId)
			: SOS::Point(rLat, rLon, rSrid)
			, mTurn(rTurn)
			, mSide(rSide)
			, mMeasTime(rMeasTime)
			, Laeq(rLaeq)
			, uncert(rUncert)
			, deviceId(rDeviceId)
			, serieId(rSerieId)
		{
		}
	};


	struct NoiseLine
	{
	
		long long lineId;

		NoiseLine()
			: lineId(LLONG_MIN)
		{
		}

		void addPoint(NoisePoint & rPoint)
		{
			std::pair<time_t, NoisePoint> record(rPoint.mMeasTime, rPoint);
			points.insert(record);
		}
	protected:
		std::multimap<time_t,NoisePoint> points;

	};

	struct StreetLine : NoiseLine
	{
		StreetLine()
			: NoiseLine()
		{
		}

		void addPoint(NoisePoint & rPoint)
		{
			NoiseLine::addPoint(rPoint);
			std::pair<time_t, NoisePoint*> record(rPoint.mMeasTime, &rPoint);

			switch(+rPoint.mSide)
			{
			
			case (+NoisePoint::Side_left) :
				leftPoints.insert(record);
				break;
			case (+NoisePoint::Side_right) :
				rightPoints.insert(record);
				break;
			case (+NoisePoint::Side_center) :
				centerPoints.insert(record);
				break;
			case (+NoisePoint::Side_unknown) :
			default:
				unknownSidePoints.insert(record);
				break;
			}
		}

	protected:
		std::multimap<time_t, NoisePoint*> leftPoints;
		std::multimap<time_t, NoisePoint*> centerPoints;
		std::multimap<time_t, NoisePoint*> rightPoints;
		std::multimap<time_t, NoisePoint*> unknownSidePoints;
	};

	struct Device
	{
		std::string name;
		long long id;
		double uncert;
	};

	struct User
	{
		std::string name;
		long long id;
		double uncert;
	};
} // namespace MobileNoise