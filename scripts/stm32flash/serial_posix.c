/*
  stm32flash - Open Source ST STM32 flash program for *nix
  Copyright (C) 2010 Geoffrey McRae <geoff@spacevs.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <sys/ioctl.h>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <assert.h>

#include "serial.h"

struct serial {
	int			fd;
	struct termios		oldtio;
	struct termios		newtio;

	char			configured;
	serial_baud_t		baud;
	serial_bits_t		bits;
	serial_parity_t		parity;
	serial_stopbit_t	stopbit;
};

serial_t* serial_open(const char *device) {
	serial_t *h = calloc(sizeof(serial_t), 1);

	h->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (h->fd < 0) {
		free(h);
		return NULL;
	}
	fcntl(h->fd, F_SETFL, 0);

	tcgetattr(h->fd, &h->oldtio);
	tcgetattr(h->fd, &h->newtio);

	return h;
}

void serial_close(serial_t *h) {
	assert(h && h->fd > -1);

	serial_flush(h);
	tcsetattr(h->fd, TCSANOW, &h->oldtio);
	close(h->fd);
	free(h);
}

void serial_flush(const serial_t *h) {
	assert(h && h->fd > -1);
	tcflush(h->fd, TCIFLUSH);
}

serial_err_t serial_setup(serial_t *h, const serial_baud_t baud, const serial_bits_t bits, const serial_parity_t parity, const serial_stopbit_t stopbit) {
	assert(h && h->fd > -1);

	speed_t		port_baud;
	tcflag_t	port_bits;
	tcflag_t	port_parity;
	tcflag_t	port_stop;

	switch(baud) {
		case SERIAL_BAUD_1200  : port_baud = B1200  ; break;
		case SERIAL_BAUD_1800  : port_baud = B1800  ; break;
		case SERIAL_BAUD_2400  : port_baud = B2400  ; break;
		case SERIAL_BAUD_4800  : port_baud = B4800  ; break;
		case SERIAL_BAUD_9600  : port_baud = B9600  ; break;
		case SERIAL_BAUD_19200 : port_baud = B19200 ; break;
		case SERIAL_BAUD_38400 : port_baud = B38400 ; break;
		case SERIAL_BAUD_57600 : port_baud = B57600 ; break;
		case SERIAL_BAUD_115200: port_baud = B115200; break;

		case SERIAL_BAUD_INVALID:
		default:
			return SERIAL_ERR_INVALID_BAUD;
	}

	switch(bits) {
		case SERIAL_BITS_5: port_bits = CS5; break;
		case SERIAL_BITS_6: port_bits = CS6; break;
		case SERIAL_BITS_7: port_bits = CS7; break;
		case SERIAL_BITS_8: port_bits = CS8; break;

		default:
			return SERIAL_ERR_INVALID_BITS;
	}

	switch(parity) {
		case SERIAL_PARITY_NONE: port_parity = 0;                       break;
		case SERIAL_PARITY_EVEN: port_parity = INPCK | PARENB;          break;
		case SERIAL_PARITY_ODD : port_parity = INPCK | PARENB | PARODD; break;

		default:
			return SERIAL_ERR_INVALID_PARITY;
	}

	switch(stopbit) {
		case SERIAL_STOPBIT_1: port_stop = 0;	   break;
		case SERIAL_STOPBIT_2: port_stop = CSTOPB; break;

		default:
			return SERIAL_ERR_INVALID_STOPBIT;
	}

	/* if the port is already configured, no need to do anything */
	if (
		h->configured        &&
		h->baud	   == baud   &&
		h->bits	   == bits   &&
		h->parity  == parity &&
		h->stopbit == stopbit
	) return SERIAL_ERR_OK;

	/* reset the settings */
	cfmakeraw(&h->newtio);
	h->newtio.c_cflag &= ~(CSIZE | CRTSCTS);
	h->newtio.c_iflag &= ~(IXON | IXOFF | IXANY | IGNPAR);
	h->newtio.c_lflag &= ~(ECHOK | ECHOCTL | ECHOKE);
	h->newtio.c_oflag &= ~(OPOST | ONLCR);

	/* setup the new settings */
	cfsetispeed(&h->newtio, port_baud);
	cfsetospeed(&h->newtio, port_baud);
	h->newtio.c_cflag |=
		port_parity	|
		port_bits	|
		port_stop	|
		CLOCAL		|
		CREAD;

	h->newtio.c_cc[VMIN ] = 0;
	h->newtio.c_cc[VTIME] = 180;

	/* set the settings */
	serial_flush(h);
	if (tcsetattr(h->fd, TCSANOW, &h->newtio) != 0)
		return SERIAL_ERR_SYSTEM;

	/* confirm they were set */
	struct termios settings;
	tcgetattr(h->fd, &settings);
	if (
		settings.c_iflag != h->newtio.c_iflag ||
		settings.c_oflag != h->newtio.c_oflag ||
		settings.c_cflag != h->newtio.c_cflag ||
		settings.c_lflag != h->newtio.c_lflag
	)	return SERIAL_ERR_UNKNOWN;

	h->configured = 1;
	h->baud	      = baud;
	h->bits	      = bits;
	h->parity     = parity;
	h->stopbit    = stopbit;
	return SERIAL_ERR_OK;
}

serial_err_t serial_write(const serial_t *h, const void *buffer, unsigned int len) {
	assert(h && h->fd > -1 && h->configured);

	ssize_t r;
	uint8_t *pos = (uint8_t*)buffer;

	while(len > 0) {
		r = write(h->fd, pos, len);
		if (r < 1) return SERIAL_ERR_SYSTEM;

		len -= r;
		pos += r;
	}

	return SERIAL_ERR_OK;
}

serial_err_t serial_read(const serial_t *h, const void *buffer, unsigned int len) {
	assert(h && h->fd > -1 && h->configured);

	ssize_t r;
	uint8_t *pos = (uint8_t*)buffer;

	while(len > 0) {
		r = read(h->fd, pos, len);
		      if (r == 0) return SERIAL_ERR_NODATA;
		else  if (r <  0) return SERIAL_ERR_SYSTEM;

		len -= r;
		pos += r;
	}

	return SERIAL_ERR_OK;
}

const char* serial_get_setup_str(const serial_t *h) {
	static char str[11];
	if (!h->configured)
		snprintf(str, sizeof(str), "INVALID");
	else
		snprintf(str, sizeof(str), "%u %d%c%d",
			serial_get_baud_int   (h->baud   ),
			serial_get_bits_int   (h->bits   ),
			serial_get_parity_str (h->parity ),
			serial_get_stopbit_int(h->stopbit)
		);

	return str;
}

serial_err_t serial_reset(const serial_t *serial, int dtr) {
    int state;
#ifdef __APPLE__
    int flush = FREAD | FWRITE;
#endif

    int err = ioctl(serial->fd, TIOCMGET, &state);
    if (err)
        return SERIAL_ERR_SYSTEM;

    // Set DTR according to the parameter
    if (dtr) {
        state &= ~TIOCM_DTR;
    } else {
        state |= TIOCM_DTR;
    }

    // Lower RTS to reset
    state |= TIOCM_RTS;
    err = ioctl(serial->fd, TIOCMSET, &state);
    if (err)
        return SERIAL_ERR_SYSTEM;

    // Wait for 10ms so that the system has time to reset
    usleep(10000);

    // Flush any pending I/O
#ifdef __APPLE__
    err = ioctl(serial->fd, TIOCFLUSH, &flush);
#else
    err = ioctl(serial->fd, TCFLSH, TCIOFLUSH);
#endif
    if (err)
        return SERIAL_ERR_SYSTEM;

    // Let it boot: Raise the RTS
    state &= ~TIOCM_RTS;
    err = ioctl(serial->fd, TIOCMSET, &state);
    if (err)
        return SERIAL_ERR_SYSTEM;

    // Wait for 100ms so that the system has time to boot up
    usleep(100000);

    return SERIAL_ERR_OK;
}
