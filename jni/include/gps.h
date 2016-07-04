/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */


#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define  LOG_TAG  "gps_glonass_serial"

#include <cutils/log.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <hardware/gps.h>

#include "misc.h"
#include "device.h"
#include "gps_status.h"
#include "interface.h"
#include "state.h"
#include "minmea.h"
#include "reader.h"
#include "notifier.h"


//#define  GPS_DEBUG 1
#undef  GPS_DEBUG

#define  DFR(...)   ALOGD(__VA_ARGS__)

#if GPS_DEBUG
#define  D(...)   ALOGD(__VA_ARGS__)
#else
#define  D(...)   ((void)0)
#endif

#define GPS_DEV_SLOW_UPDATE_RATE (100)
#define GPS_DEV_HIGH_UPDATE_RATE (1)

#define NMEA_MAX_SIZE  255
#define NMEA_STRICT false

// Default maximum refresh rate (Hz)
// It uses this value if ro.kernel.android.gps.max_rate is not set
// Typically between 1-10Hz
// Higher refresh rate cases higher CPU load
#define DEFAUTLT_REFRESH_RATE (1)