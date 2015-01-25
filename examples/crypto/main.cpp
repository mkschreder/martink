/*
Example of using aes crypto functions in libk. 

Notice that you can just as well use LibK from C++ as from C. Therefore, 
this example is in C++. 
*/ 

#include <kernel.h>
#include <crypto/aes.h>

// pin definitions
#include <boards/arduino/promini.h>

#include <string.h>

class Application {
public:
	Application(); 
	int loop(); 
private: 
	// serial device handle for the console. 
	serial_dev_t 	mConsole; 
	// timeout value for the delay
	timestamp_t 	mNextPrintTime; 
	// counter 
	int 	mCounter; 
	// aes context. Note: this is a large structure that holds the whole key
	// so expect at least 256 bytes of extra ram used. 
	aes256_ctx_t 	mAES; 
}; 

Application::Application():mConsole(0), mNextPrintTime(0), mCounter(0){
	soc_init(); 
	
	// we will be using uart device with this baud rate
	uart0_set_baudrate(38400); 
	
	// we can no get the interface for the uart0 on the chip. 
	// a serial interface can be any serial stream in libk though
	// it could have been an spi device for example but then we would have
	// used spi specific spi_get_serial_interface(); 
	mConsole = uart_get_serial_interface(0); 
	
	// we use the timestamp system to set next print time to expire right away. 
	// timestamps use hardware timer for keeping track of "ticks". Usually 
	// one or few ticks per microsecond. Therefore all timeouts have 
	// precision of up to 1 microsecond. 
	mNextPrintTime = timestamp_from_now_us(1000000UL); 
	
	serial_printf(mConsole, "BOOT: Crypto example AES\n"); 
	
	sha256_hash_t hash; 
	const char *pw = "theawesomepassword!"; 
	sha256(&hash, pw, strlen(pw)); 
	
	// has is same size as key so can use directly (256 bits)
	
	aes256_init(hash, &mAES); 
}

int Application::loop(){

	// we have here an async loop that prints a message every second
	if(timestamp_expired(mNextPrintTime)){
		char block[AES_BLOCK_SIZE]; 
		snprintf(block, sizeof(block) - 1, "Msg nr %d", mCounter++); 
		serial_printf(mConsole, "Encrypting: %s\n", block); 
		aes256_enc(block, &mAES);
		serial_printf(mConsole, "Encrypted message: "); 
		for(int c = 0; c < sizeof(block)-1; c++){
			serial_printf(mConsole, "%02x ", (int)block[c] & 0xff); 
		}
		serial_printf(mConsole, "\n"); 
		
		aes256_dec(block, &mAES); 
		serial_printf(mConsole, "Decrypted hex: "); 
		for(int c = 0; c < sizeof(block)-1; c++){
			serial_printf(mConsole, "%02x ", (int)block[c] & 0xff); 
		}
		serial_printf(mConsole, "\n"); 
		block[sizeof(block) - 1] = 0; 
		serial_printf(mConsole, "Decrypted string: %s\n", block); 
		serial_printf(mConsole, "----------\n"); 
		// set one second timeout until next time
		mNextPrintTime = timestamp_from_now_us(1000000UL); 
	}
}

int main(void){
	Application app; 
	
	while(1){
		app.loop(); 
	}
}
