CREATE OR REPLACE FUNCTION make_series(min_pause INTERVAL, device TEXT, _tbl regclass)RETURNS void LANGUAGE plpgsql AS
$func$
DECLARE
	RECORD meas_id_time;
	INT last_ser_id
	TIMESTAMP first_phen_time;
	TIMESTAMP last_phen_time;
BEGIN
	RAISE NOTICE 'Making series for %s ...', quote_ident(device);

	SELECT phenom_time INTO STRICT first_phen_time format('FROM measurements WHERE procedure = %s ORDER BY phenom_time DESC LIMIT 1', device);

	IF EXISTS (format('SELECT 1 FROM measurements WHERE phenom_time < first_phen_time AND procedure = %s ', device)) THEN

		SELECT series_id INTO STRICT last_ser_id format('FROM measurements WHERE procedure = %s AND phenom_time < first_phen_time ORDER BY phenom_time DESC LIMIT 1', device);
		SELECT phenom_time INTO STRICT last_phen_time format('FROM measurements WHERE procedure = %s AND phenom_time < first_phen_time ORDER BY phenom_time DESC LIMIT 1', device);
	ELSE
		last_ser_id := 0;
		last_phen_time := first_phen_time;
		
	END IF;

	FOR meas_id_time IN format('SELECT * FROM %s WHERE procedure = %s ORDER BY phenom_time', _tbl, device) LOOP
	
		IF (meas_id_time.phenom_time - last_phen_time) > min_pause THEN

			last_ser_id := last_ser_id + 1;

		END IF;

		UPDATE measurements SET series_id = last_ser_id WHERE id = meas_id_time.id;
		last_phen_time := meas_id_time.phenom_time;

		
	END LOOP;
	

END
$func$;

CREATE OR REPLACE FUNCTION determine_series(min_pause INTERVAL, _tbl regclass)
RETURNS void LANGUAGE plpgsql AS
$func$
DECLARE
	RECORD devices;
BEGIN
	RAISE NOTICE 'Determining series ...';

	FOR meas_id_time IN format('SELECT procedure FROM %s GROUP BY procedure', _tbl) LOOP
	
		PERFORM make_series(min_pause, meas_id_time.procedure, _tbl);
		
	END LOOP;

END
$func$;

PERFORM determine_series('00:00:20', 'measurements'::regclass);