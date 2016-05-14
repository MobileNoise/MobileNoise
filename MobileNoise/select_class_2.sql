--SELECT result from res_geom, measurements where id_meas = measurements.id AND uom = '4326'
--UPDATE measurements SET foi_geometry = result FROM res_geom WHERE measurements.id = id_meas AND uom = '4326'
--SELECT ST_SetSRID(ST_Extent(the_geom),THE_SRID) as table_extent FROM your_table;
--SELECT ST_SetSRID(ST_Extent(measurements.foi_geometry),4326) AS bbox INTO bbox_bbox FROM measurements;
SELECT ST_AsEWKT(bbox) from bbox_bbox LIMIT 1;
--DROP table bbox_bbox