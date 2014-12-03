deps_config := \
	tty/KConfig \
	sensors/KConfig \
	rfid/KConfig \
	radio/KConfig \
	net/KConfig \
	motors/KConfig \
	io/KConfig \
	hid/KConfig \
	disp/KConfig \
	crypto/KConfig \
	Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
