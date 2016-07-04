/*
 * Copyright © 2015 Denys Petrovnin <dipcore@gmail.com>
 */


#include "gps.h"

/* commands sent to the gps thread */
enum {
    CMD_QUIT  = 0,
    CMD_START = 1,
    CMD_STOP  = 2
};

/*
void gps_state_lock_fix(GpsState *state) {
    int ret;
    do {
        ret=sem_wait(&state->fix_sem);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0) {
        D("Error in GPS state lock:%s\n", strerror(errno));
    }
}

void gps_state_unlock_fix(GpsState *state) {
    if (sem_post(&state->fix_sem) == -1)
	{
		if(errno == EAGAIN)
			if(sem_post(&state->fix_sem)== -1)
				D("Error in GPS state unlock:%s\n", strerror(errno));
	}
}
*/

void gps_state_done( GpsState*  s )
{
    // tell the thread to quit, and wait for it
    char   cmd = CMD_QUIT;
    void*  dummy;
    write( s->control[0], &cmd, 1 );
    pthread_join(s->thread, &dummy);

    // close the control socket pair
    close( s->control[0] ); s->control[0] = -1;
    close( s->control[1] ); s->control[1] = -1;

    // close connection to the QEMU GPS daemon
    close( s->fd ); s->fd = -1;
    s->init = 0;
}


void gps_state_start( GpsState*  s )
{
    char  cmd = CMD_START;
    int   ret;

    do {
        ret = write( s->control[0], &cmd, 1 );
    } while (ret < 0 && errno == EINTR);

    if (ret != 1)
        D("%s: could not send CMD_START command: ret=%d: %s",
                __FUNCTION__, ret, strerror(errno));
}


void gps_state_stop( GpsState*  s )
{
    char  cmd = CMD_STOP;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        D("%s: could not send CMD_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}


static int epoll_register( int  epoll_fd, int  fd )
{
    struct epoll_event  ev;
    int ret, flags;

    /* important: make the fd non-blocking */
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
    } while (ret < 0 && errno == EINTR);
    return ret;
}


static int epoll_deregister( int  epoll_fd, int  fd )
{
    int  ret;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, NULL );
    } while (ret < 0 && errno == EINTR);
    return ret;
}


/* this is the main thread, it waits for commands from gps_state_start/stop and,
 * when started, messages from the QEMU GPS daemon. these are simple NMEA sentences
 * that must be parsed to be converted into GPS fixes sent to the framework
 */
void gps_state_thread( void*  arg )
{
    GpsState*   state = (GpsState*) arg;
    int         epoll_fd   = epoll_create(2);
    int         started    = 0;
    int         gps_fd     = state->fd;
    int         control_fd = state->control[1];

    // register control file descriptors for polling
    epoll_register( epoll_fd, control_fd );
    epoll_register( epoll_fd, gps_fd );

    gps_dev_init(gps_fd);

    D("GPS thread running");

    // now loop
    for (;;) {
        struct epoll_event   events[2];
        int                  ne, nevents;

        nevents = epoll_wait( epoll_fd, events, 2, -1 );
        if (nevents < 0) {
            if (errno != EINTR)
                ALOGE("epoll_wait() unexpected error: %s", strerror(errno));
            continue;
        }
        for (ne = 0; ne < nevents; ne++) {
            if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
                ALOGE("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                gps_dev_deinit(gps_fd);
                return;
            }
            if ((events[ne].events & EPOLLIN) != 0) {
                int  fd = events[ne].data.fd;

                if (fd == control_fd) {
                    char  cmd = 255;
                    int   ret;
                    D("GPS control fd event");
                    do {
                        ret = read( fd, &cmd, 1 );
                    } while (ret < 0 && errno == EINTR);

                    if (cmd == CMD_QUIT) {
                        D("GPS thread quitting on demand");
                        gps_dev_deinit(gps_fd);
                        return;
                    } else if (cmd == CMD_START) {
                        if (!started) {
                            D("GPS thread starting  location_cb=%p", state->callbacks->location_cb);
                            started = 1;
                            gps_dev_start(gps_fd);                            
                        }
                    } else if (cmd == CMD_STOP) {
                        if (started) {
                            D("GPS thread stopping");
                            started = 0;
                            gps_dev_stop(gps_fd);
                        }
                    }
                } else if (fd == gps_fd) {
                    char  buff[32];
                    for (;;) {
                        int  nn, ret;

                        ret = read( fd, buff, sizeof(buff) );
                        if (ret < 0) {
                            if (errno == EINTR)
                                continue;
                            if (errno != EWOULDBLOCK)
                                ALOGE("Error while reading from GPS daemon socket: %s:", strerror(errno));
                            break;
                        }
                        nmea_reader_append( buff, ret );
                    }
                } else {
                    ALOGE("epoll_wait() returned unkown fd %d ?", fd);
                }
            }
        }
    }
}


void gps_state_init( GpsState*  state, GpsCallbacks* callbacks )
{
    char   prop[PROPERTY_VALUE_MAX];
    char   baud[PROPERTY_VALUE_MAX];
    char   device[256];
    int    ret;
    int    done = 0;

    char   max_refresh_rate_str[255];


    struct sigevent tmr_event;

    state->init       = 1;
    state->control[0] = -1;
    state->control[1] = -1;
    state->fd         = -1;
    state->callbacks  = callbacks;
    D("gps_state_init");

    // Look for a kernel-provided maximum refresh rate 
    if (property_get("ro.kernel.android.gps.max_rate",max_refresh_rate_str, "") == 0){
    	D("no kernel-provided max refresh rate, using default: %d", DEFAUTLT_REFRESH_RATE);
    	max_refresh_rate = DEFAUTLT_REFRESH_RATE;
    } else {
    	sscanf (max_refresh_rate_str,"%d",&max_refresh_rate);
    }

    // Look for a kernel-provided device name
    if (property_get("ro.kernel.android.gps",prop,"") == 0) {
        D("no kernel-provided gps device name");
        return;
    }

    snprintf(device, sizeof(device), "/dev/%s",prop);
    do {
        //state->fd = open( device, O_RDWR | O_NOCTTY | O_NONBLOCK);
        state->fd = open( device, O_RDWR);
    } while (state->fd < 0 && errno == EINTR);

    if (state->fd < 0) {
        ALOGE("could not open gps serial device %s: %s", device, strerror(errno) );
        return;
    }

    D("GPS will read from %s", device);

    // Disable echo on serial lines
    if ( isatty( state->fd ) ) {
        struct termios  ios;
        tcgetattr( state->fd, &ios );
        ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
        ios.c_oflag &= (~ONLCR); /* Stop \n -> \r\n translation on output */
        ios.c_iflag &= (~(ICRNL | INLCR)); /* Stop \r -> \n & \n -> \r translation on input */
        ios.c_iflag |= (IGNCR | IXOFF);  /* Ignore \r & XON/XOFF on input */
        //ios.c_cc[VMIN]=1; // one byte
        //ios.c_cc[VTIME]=10; // 0.1s
		// Set baud rate and other flags
        property_get("ro.kernel.android.gps.speed",baud,"9600");
	if (strcmp(baud, "4800") == 0) {
            ALOGE("Setting gps baud rate to 4800");
            ios.c_cflag = B4800 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(baud, "9600") == 0) {
            ALOGE("Setting gps baud rate to 9600");
            ios.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(baud, "19200") == 0) {
            ALOGE("Setting gps baud rate to 19200");
            ios.c_cflag = B19200 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(baud, "38400") == 0) {
            ALOGE("Setting gps baud rate to 38400");
            ios.c_cflag = B38400 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(baud, "57600") == 0) {
            ALOGE("Setting gps baud rate to 57600");
            ios.c_cflag = B57600 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else if (strcmp(baud, "115200") == 0) {
            ALOGE("Setting gps baud rate to 115200");
            ios.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
        } else {
            ALOGE("GPS baud rate unknown: '%s'", baud);
            return;
        }

        tcsetattr( state->fd, TCSANOW, &ios );
        //tcflush( state->fd, TCIFLUSH );      /* Flush */
    }

    if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, state->control ) < 0 ) {
        ALOGE("Could not create thread control socket pair: %s", strerror(errno));
        goto Fail;
    }

    state->thread = callbacks->create_thread_cb( "gps_state_thread", gps_state_thread, state );

    if ( !state->thread ) {
        ALOGE("Could not create GPS thread: %s", strerror(errno));
        goto Fail;
    }

    D("GPS state initialized");

    return;

Fail:
    gps_state_done( state );
}
