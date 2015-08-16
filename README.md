# gps-glonass-android-driver
**GPS/GLONASS** generic android driver for serial devices. (gps.default.so)

It uses **ro.kernel.android.gps**, **ro.kernel.android.gps.speed** and **ro.kernel.android.gps.max_rate** kernel parameters. 

build.prop example:
`````
  ro.kernel.android.gps=ttyUSB0
  ro.kernel.android.gps.speed=9600
  ro.kernel.android.gps.max_rate=1
```

## Description
* **ro.kernel.android.gps**         - [REQUIRED]      Device name
* **ro.kernel.android.gps.speed**   - [NON REQUIRED]  Baud rate. Supports: 4800, 9600, 19200, 38400, 57600, 115200. Default value: 9600
* **ro.kernel.android.gps.max_rate**   - [NON REQUIRED]  Maximum refresh rate. Between 1-10Hz. Default value: 1

You do not need to set baudrate and echo with **stty** tool, the library does it for you.

**IMPORTANT** Higher max refresh rate causes higher CPU load. Recommended value for usual cases is 1Hz (car navigation, etc)
