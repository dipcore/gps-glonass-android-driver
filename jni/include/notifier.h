/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

 static GpsSvStatus sv_status;
 static GpsLocation location;
 static int sv_counter;
 static int svs_num_counter;
 static char oldTalker[3];

 void notifier_append_sv(char talker[3], int prn, float elevation, float azimuth, float snr);
 void notifier_svs_inview(char talker[3], int num_svs);
 void notifier_svs_update_status(char talker[3], int msg_number, int total_msgs);
 void notifier_set_speed(float speed_knots);
 void notifier_set_latlong(double lat, double lon);
 void notifier_set_altitude(double altitude, char units);
 void notifier_set_accuracy(float accuracy);
 void notifier_push_location();