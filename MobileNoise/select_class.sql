﻿--SELECT ST_AsText(ST_Transform(way,4326)) from planet_osm_line WHERE osm_id = 31280027
--SELECT * FROM  planet_osm_line WHERE planet_osm_line.way && ST_Transform(ST_Expand(ST_GeomFromEWKT(" << bbox << "),0.0001),900913);
--SELECT ST_Transform(ST_Expand(ST_GeomFromEWKT('SRID=4326;POLYGON((16.0 49.999,16.0 50.0, 16.001 50.0,16.1 49.999, 16.0 49.999))'),0.0001),900913) AS bbox INTO bbox_bbox;
--SELECT clipped.osm_id, clipped_geom INTO bbox_bbox_geo
--FROM (SELECT planet_osm_line.osm_id, (ST_Dump(ST_Intersection(bbox_bbox.bbox, planet_osm_line.way))).geom As clipped_geom
--FROM bbox_bbox
--	INNER JOIN planet_osm_line
--	ON ST_Intersects(bbox_bbox.bbox, planet_osm_line.way))  
--As clipped
--WHERE ST_Dimension(clipped.clipped_geom) = 1 ;
--SELECT * FROM bbox_bbox_geo
--ALTER TABLE bbox_bbox_geo ADD COLUMN id SERIAL PRIMARY KEY;
--SELECT osm_id, ST_AsGeoJSON((ST_Segmentize(clipped_geom::geography, 5)),15, 2), id FROM bbox_bbox_geo WHERE id > -1 ORDER BY id ASC LIMIT 100;
--ALTER TABLE bbox_bbox_geo ALTER COLUMN clipped_geom TYPE Geometry(Linestring, 4326) USING ST_Transform(clipped_geom, 4326);
--SELECT ST_AsText(bbox) FROM bbox_bbox;
--SELECT EXISTS (SELECT bbox FROM bbox_bbox WHERE ST_DWithin(bbox_bbox.bbox, ST_SetSRID(ST_MakePoint(1608977.82766415, 4999896.03901706), 900913), 100000000));
--SELECT bbox FROM bbox_bbox WHERE ST_DWithin(bbox_bbox.bbox, ST_SetSRID(ST_MakePoint(1608977.82766415, 4999896.03901706), 900913), 10000000) ORDER BY ST_Distance(bbox_bbox.bbox, ST_SetSRID(ST_MakePoint(1608977.82766415, 4999896.03901706), 900913)) LIMIT 1;
--SELECT ST_AsText(ST_Transform(ST_GeometryFromText('POINT(1608977.82766415 4999896.03901706)',900913), 4326));
--SELECT ST_AsText(ST_Transform(ST_GeometryFromText('POINT(14.453 40.915)',4326), 4326));
SELECT * from planet_osm_line WHERE osm_id = 0;