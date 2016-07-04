/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

int max_refresh_rate;
static GpsSvStatus sv_status;
static GpsLocation location;
static int sv_counter;
static int svs_used_ids[12];
static char oldTalker[3];
static int utc_diff;
static int64_t last_location_cycle_timestamp;
static int64_t last_svs_cycle_timestamp;

void notifier_init_utc_diff();

void notifier_svs_append(char talker[3], int prn, float elevation, float azimuth, float snr);
void notifier_svs_inview(char talker[3], int num_svs);
void notifier_svs_update_status();
void notifier_svs_used_ids(int ids[12]);

void notifier_set_speed(float speed_knots);
void notifier_set_bearing(float bearing);
void notifier_set_latlong(double lat, double lon);
void notifier_set_altitude(double altitude, char units);
void notifier_set_accuracy(float accuracy);
void notifier_set_date_time(struct minmea_date date, struct minmea_time time_);
void notifier_push_location();