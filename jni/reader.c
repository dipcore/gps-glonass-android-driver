/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

#include "gps.h"

static char line[ NMEA_MAX_SIZE + 1 ];
static bool overflow;
static int pos;

void nmea_reader_append (char *buff, int size)
{
	for (int n = 0; n < size; n++){

		if (overflow) {
	        overflow = (buff[n] != '\n');
	        return;
	    }

	    if (pos >= NMEA_MAX_SIZE ) {
	        overflow = true;
	        pos = 0;
	        return;
	    }

		line[pos] = (char) buff[n];
	    pos++;

	    if (buff[n] == '\n') {
	    	line[pos] = '\0';
	        nmea_reader_parse(line);
	        pos = 0;
	    }

	}
}

void nmea_reader_parse(char *line) {
	switch (minmea_sentence_id(line, NMEA_STRICT)) {
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame;
            if (minmea_parse_rmc(&frame, line)) {
                D("$RMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
                        frame.latitude.value, frame.latitude.scale,
                        frame.longitude.value, frame.longitude.scale,
                        frame.speed.value, frame.speed.scale);
                D("$RMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
                        minmea_rescale(&frame.latitude, 1000),
                        minmea_rescale(&frame.longitude, 1000),
                        minmea_rescale(&frame.speed, 1000));
                D("$RMC floating point degree coordinates and speed: (%f,%f) %f\n",
                        minmea_tocoord(&frame.latitude),
                        minmea_tocoord(&frame.longitude),
                        minmea_tofloat(&frame.speed));
            }
        } break;

        case MINMEA_SENTENCE_GGA: {
            struct minmea_sentence_gga frame;
            if (minmea_parse_gga(&frame, line)) {
                D("$GGA: fix quality: %d\n", frame.fix_quality);
            }
        } break;

        case MINMEA_SENTENCE_GSV: {
            struct minmea_sentence_gsv frame;
            if (minmea_parse_gsv(&frame, line)) {
                D("$GSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
                D("$GSV: sattelites in view: %d\n", frame.total_sats);
                for (int i = 0; i < 4; i++)
                    D("$GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
                        frame.sats[i].nr,
                        frame.sats[i].elevation,
                        frame.sats[i].azimuth,
                        frame.sats[i].snr);
            }
        } break;
    }
}