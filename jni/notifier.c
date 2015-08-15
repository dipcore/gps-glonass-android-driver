/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

#include"gps.h"

void notifier_append_sv(char talker[3], int prn, float elevation, float azimuth, float snr) {
	sv_status.sv_list[sv_counter].prn = prn;
	sv_status.sv_list[sv_counter].elevation = elevation;
	sv_status.sv_list[sv_counter].azimuth = azimuth;
	sv_status.sv_list[sv_counter].snr = snr;
	sv_counter ++;
}

void notifier_svs_inview(char talker[3], int num_svs) {
	if (strcmp(talker, oldTalker)) {
		svs_num_counter += num_svs;
		memcpy(oldTalker, talker, 3);
	}
	sv_status.num_svs = svs_num_counter;    
}

void notifier_svs_update_status(char talker[3], int msg_nr, int total_msgs) {
	if (msg_nr == 1 && !strcmp(talker, "GP")) {
		// update
		update_gps_svstatus(&sv_status);
		// start new sv cycle
		sv_counter = 0;
		svs_num_counter = 0;
	}
}

void notifier_set_speed(float speed_knots) {
	location.flags   |= GPS_LOCATION_HAS_SPEED;
	location.speed    = speed_knots * 1.852 / 3.6; // knots to m/s
}

void notifier_set_latlong(double lat, double lon) {
	location.flags    |= GPS_LOCATION_HAS_LAT_LONG;
	location.latitude  = lat;
	location.longitude = lon;
}

void notifier_set_altitude(double altitude, char units) {
	location.flags   |= GPS_LOCATION_HAS_ALTITUDE;
	location.altitude = altitude;
}

void notifier_set_accuracy(float accuracy) {
	location.flags   |= GPS_LOCATION_HAS_ACCURACY;
	location.accuracy = accuracy;
}

void notifier_push_location() {
	if (location.flags & GPS_LOCATION_HAS_ACCURACY) {
		update_gps_location(&location);
	}
}

