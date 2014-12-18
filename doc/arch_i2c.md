I2C / two wire interface
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/<manufacturer>/<model>/twi.h
* arch/twi.h

Public interface
----------------

The public interface used by the i2c bus is packet_interface. This is because i2c is a protocol based state machine that expects some standard data to operate. In this respect it operates just like any packet interface - it sends data buffers and receives data into a buffer. One special case that takes the point home is this: i2c devices sometimes expect a write operation followed by a repeated start signal and then a read operation. This sequence is used to read values from an i2c device. Using packet interface allows us to model this behaviour since it has begin(), end() write() and read() methods. Serial interface would be inappropriate here.


| Interface method | Corresponding twi action |
|------------------|--------------------------|
void 					(*begin)(struct packet_interface *self) | must be called every time you want to start sending data. This call allows the driver to power on the device and prepare it for accepting new data.
void 					(*end)(struct packet_interface *self) | must be called when you do not intend to send more data for a while. This method signals the driver to release the bus to other devices since packet interfaces usually share underlying medium. It is also a good place to tell the other side that we are done transmitting data. 
uint32_t			(*write)(struct packet_interface *self, const uint8_t *data, uint16_t max_sz) | writes a packet to the device the max_sz parameter tells the device how big the packet is. The function can return before the data is actually sent. In that case the device will be busy and will not accept any data until the previous data has been sent
uint32_t			(*read)(struct packet_interface *self, uint8_t *data, uint16_t max_sz) | reads a packet from the device. May return before the operation completes! max_sz denotes maximum number of bytes that are read. If a packet that arrives in hardware is larger than max_sz then the function reads in max_sz bytes and returns an error in top 16 bits of the return value. The bottom 16 bits are number of bytes that have been read. You must always check status to see if previous operation has been completed before you make any changes to passed parameters because the method accesses the data directly and if you for example exit from a function where data is a stack variable without making sure that the operation has completed, the result may be a crash because the driver is not mandated to block until the data has been received. 
void					(*sync)(struct packet_interface *self) | instructs the hardware to send all the data it has in it's buffers and waits for the hardware to complete as well as makes sure that all pending operations have completed before exiting. 
uint16_t 			(*packets_available)(struct packet_interface *self) | returns the number of packets that are waiting to be read using getn. If a read operation is already in progress, this call should return one less packets or zero in the case where only one packet can be received at a time. 


Implementation macro guidelines
----------------------

| Hardware method | Description |
|-----------------|-------------|
twi0_init(speed) | initializes twi peripheral to run at given clock speed
twi0_begin() | must be called when user is about to send a packet. 
twi0_end(void) | must be called when you do not intend to use the peripheral any more. It also waits for all previous operations to finish before returning so it can be used as way to sync with the peripheral. 
twi0_start_write(uint8_t *data, uint8_t data_sz) | starts a write operation on the bus. This method may return directly but the operation may be done in the background. The buffer that is supplied is used directly in order to save memory so it must not be destroyed! 
twi0_start_read(uint8_t *data, uint8_t data_sz) | starts a read operation and works much like the start_read() method. 
twi0_busy(void) | checks if twi is busy processing a transaction
twi0_success(void) | checks whether last operation completed successfully

Example usage
--------------------

```
char buffer[] = {DEVICE_ADDRESS, 'H', 'e', 'l', 'l', 'o'}; 
twi0_init(100000);
twi0_begin();
twi0_start_write(buffer, 6);
twi0_start_read(buffer, 6); // will read data into buffer starting at buffer[1].. 
twi0_end()
```
