# gps-glonass-android-driver
**GPS/GLONASS** generic android driver for serial devices. (gps.default.so)

It uses **ro.kernel.android.gps**, **ro.kernel.android.gps.speed** and **ro.kernel.android.gps.max_rate** kernel parameters. 

## Installation
Just copy the library to **/system/lib/hw/** folder add properties to **build.prop** and reboot device.

## Kernel properties
### build.prop example:
```
  ro.kernel.android.gps=ttyUSB0
  ro.kernel.android.gps.speed=9600
  ro.kernel.android.gps.max_rate=1
```
### Properties description
* **ro.kernel.android.gps**         - [REQUIRED]      Device name
* **ro.kernel.android.gps.speed**   - [NON REQUIRED]  Baud rate. Supports: 4800, 9600, 19200, 38400, 57600, 115200. Default value: 9600
* **ro.kernel.android.gps.max_rate**   - [NON REQUIRED]  Maximum refresh rate. Between 1-10Hz. Default value: 1

**IMPORTANT** Higher max refresh rate causes higher CPU load. Recommended value is 1Hz (car navigation, etc)

## init.d script
On some devices the library cannot set baudrate itself, in that case you can use the folowing script. Call it *00_gps.sh*, save it in *init.d* folder, add *755* permisions and *root* as an owner.

```
#!/system/bin/sh
# GPS Switcher
# init.d 00_gps.sh
# dipcore@gmail.com

GPSTTY=/dev/`getprop ro.kernel.android.gps`
SPEED=`getprop ro.kernel.android.gps.speed`

if [[ ! -z $GPSTTY ]] ; then

	# Default speed
	if [[ -z $SPEED ]] ; then
		SPEED="9600"
	fi;
	
	# Serial device params
	if [ -c $GPSTTY ] ; then
		echo "Setting USB GPS $GPSTTY speed to $SPEED"
		# Speed
		busybox stty -F $GPSTTY ispeed $SPEED
		# No echo
		busybox stty -F $GPSTTY -echo
		# Owner
		chown root:system $GPSTTY
		# Permissions
		chmod 666 $GPSTTY
	fi;
	
fi;
```
*** busybox required
