Device interfaces
=================


How to device access works
--------------------------

For the sake of being light weight, the on chip devices all have static methods that are used for accessing each device. You can use macros to construct code for accessing a low level on chip device and most of these macros are device in arch/*.h files.

Thus we can write spi0_init() and spi0_writereadbyte() to interact with the spi0 interface on the chip. All this is fine, until we want to configure another device driver to use a speciffic spi device. Of course we could write the driver so that it has the spi device hardcoded and always uses one speciffic output device - this is fine if we only have one instance of the driver - but boards can have several instances of the same device connected to completely different spi ports. And this needs to be configured.

This does require some kind of way to reference an on chip device that can be specified dynamically in code. Generating separate code for each instance of the device no longer provides much benefit - so we need to use some extra memory to do this. The way this is solved right now is through the use of interfaces. Every type of low level device has an interface struct that can be optionally used to access that device from a driver. This interface we can construct dynamically. The overhead is a little memory used for storing the function pointers and a small overhead of calling a function pointer but this is negligible unless you have some really really small device. And also the library allows you to actually not use this interface at all if you don't want to - however higher level drivers use it because it adds a configurability benefit to the driver so that we can easily define multiple instances of a driver for a board.

All device instance data is stored in a device specific struct along with any interfaces to lower level devices that the driver uses to talk to the device. This way we can configure a device in the board file to use i2c port X and address Y to talk to a device and then the driver code can use the constructed interface to access the device without knowing exactly which i2c port and address it is using. The driver does not need this data - it only needs a reference to an i2c interface that is already preconfigured to use specific settings for that board.

The kind of code structure we are aiming to achieve is that all on board devices have to be configured in the board driver. All application code then uses structs defined in the board struct to talk to all devices accessible to it. 
