/*
 * Copyright © 2014 Denys Petrovnin <dipcore@gmail.com>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See the COPYING file for more details.
 */

static void gps_dev_power(int state)
{
    return;
}


static void gps_dev_send(int fd, char *msg)
{
    int i, n, ret;

    i = strlen(msg);

    n = 0;

    do {

        ret = write(fd, msg + n, i - n);

        if (ret < 0 && errno == EINTR) {
            continue;
        }

        n += ret;

    } while (n < i);
}


static unsigned char gps_dev_calc_nmea_csum(char *msg)
{
    unsigned char csum = 0;
    int i;

    for (i = 1; msg[i] != '*'; ++i) {
        csum ^= msg[i];
    }

    return csum;
}


static void gps_dev_set_nmea_message_rate(int fd, char *msg, int rate)
{
    char buff[50];
    int i;

    sprintf(buff, "$PUBX,40,%s,%d,%d,%d,0*", msg, rate, rate, rate);

    i = strlen(buff);

    sprintf((buff + i), "%02x\r\n", gps_dev_calc_nmea_csum(buff));

    gps_dev_send(fd, buff);

    D("GPS sent to device: %s", buff);
}


static void gps_dev_set_baud_rate(int fd, int baud)
{
    char buff[50];
    int i, u;

    for (u = 0; u < 3; ++u) {

        sprintf(buff, "$PUBX,41,%d,0003,0003,%d,0*", u, baud);

        i = strlen(buff);

        sprintf((buff + i), "%02x\r\n", gps_dev_calc_nmea_csum(buff));

        gps_dev_send(fd, buff);

        D("Sent to device: %s", buff);

    }
}


static void gps_dev_set_message_rate(int fd, int rate)
{

    unsigned int i;

    char *msg[] = {
                     "GGA", "GLL", "VTG",
                     "GSA", "GSV", "RMC"
                  };

    for (i = 0; i < sizeof(msg)/sizeof(msg[0]); ++i) {
        gps_dev_set_nmea_message_rate(fd, msg[i], rate);
    }

    return;
}


static void gps_dev_init(int fd)
{
    gps_dev_power(1);

    return;
}


static void gps_dev_deinit(int fd)
{
    gps_dev_power(0);
}


static void gps_dev_start(int fd)
{
    // Set full message rate
    gps_dev_set_message_rate(fd, GPS_DEV_HIGH_UPDATE_RATE);

    D("GPS dev start initiated");
}


static void gps_dev_stop(int fd)
{
    // Set slow message rate
    gps_dev_set_message_rate(fd, GPS_DEV_SLOW_UPDATE_RATE);

    D("GPS dev stop initiated");
}