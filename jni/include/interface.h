/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

int serial_gps_init(GpsCallbacks* callbacks);
void serial_gps_cleanup(void);
int serial_gps_start();
int serial_gps_stop();
int serial_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty);
int serial_gps_inject_location(double latitude, double longitude, float accuracy);
void serial_gps_delete_aiding_data(GpsAidingData flags);
int serial_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence, 
		uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time);
const void* serial_gps_get_extension(const char* name);