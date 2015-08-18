/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

#include "gps.h"

int serial_gps_init(GpsCallbacks* callbacks) {
    D("serial_gps_init");
    GpsState*  s = _gps_state;

    if (!s->init)
        gps_state_init(s, callbacks);

    if (s->fd < 0)
        return -1;

    return 0;
}


void serial_gps_cleanup(void) {
    GpsState*  s = _gps_state;

    if (s->init)
        gps_state_done(s);
}


int serial_gps_start() {
    GpsState*  s = _gps_state;

    if (!s->init) {
        DFR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    D("%s: called", __FUNCTION__);
    gps_state_start(s);
    return 0;
}


int serial_gps_stop() {
    GpsState*  s = _gps_state;

    if (!s->init) {
        DFR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    D("%s: called", __FUNCTION__);
    gps_state_stop(s);
    return 0;
}


int serial_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty) {
    return 0;
}


int serial_gps_inject_location(double latitude, double longitude, float accuracy) {
    return 0;
}

void serial_gps_delete_aiding_data(GpsAidingData flags) {
}

int serial_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
        uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time) {
    GpsState*  s = _gps_state;

    if (!s->init) {
        D("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    D("set_position_mode: mode=%d recurrence=%d min_interval=%d preferred_accuracy=%d preferred_time=%d",
            mode, recurrence, min_interval, preferred_accuracy, preferred_time);

    return 0;
}


const void* serial_gps_get_extension(const char* name) {
	D("%s: no GPS extension for %s is found", __FUNCTION__, name);
    return NULL;
}
