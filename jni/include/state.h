/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

typedef struct {
    int                     init;
    int                     fd;
    GpsCallbacks            *callbacks;
    GpsStatus               status;
    pthread_t               thread;
    int                     control[2];
} GpsState;

GpsState  _gps_state[1];

void gps_state_init( GpsState*  state, GpsCallbacks* callbacks );
void gps_state_thread( void*  arg );
void gps_state_stop( GpsState*  s );
void gps_state_start( GpsState*  s );
void gps_state_done( GpsState*  s );
