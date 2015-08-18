/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */

#include <gps.h>

#ifdef __MINGW32__
#define gmtime_r(s1,s2) gmtime(s1)
#endif

#ifndef HAVE_TIMEGM

time_t timegm(struct tm *tm) {
  time_t temp_ltime;
  struct tm temp_gm;

  if (!tm) {
    temp_ltime = 0;
  }
  else {
    temp_ltime = mktime(tm);
  }

  gmtime_r(&temp_ltime, &temp_gm);

  return (time_t)(temp_ltime + (temp_ltime - mktime(&temp_gm)));
}

#endif /* HAVE_TIMEGM */