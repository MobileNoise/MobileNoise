#include "MobileNoise.h"

int main(void)
{
	MobileNoise::SOS::SosReader r;

	static const std::string mobNoiseConnStr = "dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432";
	static const std::string mobNoiseTableName = "measurements";
	static const time_t minIntervalBetweenSeries_s = 20;

	int status = 0;
	//status = r.pullAllData(mobNoiseConnStr, "http://localhost:8080/52n-sos-webapp/service", "GetObservationsNoise.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise");
	
	MobileNoise::NoiseMeterClassifier nmc;
	status = nmc.runClassification(mobNoiseConnStr, mobNoiseTableName, minIntervalBetweenSeries_s);



	/*int status = r.pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432", "http://localhost:8080/52n-sos-webapp/service", "GetObservationsNoise.json", "N/A");
	status = r.pullData<MobileNoise::SOS::OM_Measurement<MobileNoise::SOS::Point, MobileNoise::SOS::Point>>("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432", "http://localhost:8080/52n-sos-webapp/service", "GetObservationsLocation.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "4326");
	status = r.pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432", "http://localhost:8080/52n-sos-webapp/service", "GetObservationsStreetSide.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_turn");
	status = r.pullData<MobileNoise::SOS::OM_Measurement<double, MobileNoise::SOS::Point>>("dbname=MobNoise user=postgres password=shift hostaddr=127.0.0.1 port=5432", "http://localhost:8080/52n-sos-webapp/service", "GetObservationsTurn.json", "N/A", "http://sweet.jpl.nasa.gov/2.3/phenWaveNoise.owl#Noise", "degrees_street_side");*/


	return status;
}