/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

#include "gps.h"

void update_gps_status(GpsStatusValue val)
{
    GpsState* state = _gps_state;
    //Should be made thread safe...
    state->status.status=val;
    if (state->callbacks->status_cb)
        state->callbacks->status_cb(&state->status);
}

void update_gps_svstatus(GpsSvStatus *val)
{
    GpsState* state = _gps_state;
    //Should be made thread safe...
    if (state->callbacks->sv_status_cb)
        state->callbacks->sv_status_cb(val);
}

void update_gps_location(GpsLocation *fix)
{
    GpsState* state = _gps_state;
    //Should be made thread safe...
    if (state->callbacks->location_cb)
        state->callbacks->location_cb(fix);
}