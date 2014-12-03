/*
* SPI Expansion bus driver - addressable spi bus with 1Mbs throughput at 16Mhz F_CPU
*
* Martin Schröder, info@fortmax.se
* http://github.com/mkschreder
* 
* Released under GPLv3
*/

#include "avrbus.h" 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h> 

#define SPI_is_master (SPCR & _BV(MSTR))
#define SPI_is_slave (!SPI_is_master)

#define SShi {SPI_PORT |= _BV(SPI_SS);}
#define SSlo {SPI_PORT &= ~_BV(SPI_SS);}

#define CEhi {SPI_PORT |= _BV(SPI_CE);}
#define CElo {SPI_PORT &= ~_BV(SPI_CE);}

#define SSout {SPI_DDR |= _BV(SPI_SS);}
#define SSin {SPI_DDR &= ~_BV(SPI_SS);}

#define CEout {SPI_DDR |= _BV(SPI_CE);}
#define CEin {SPI_DDR &= ~_BV(SPI_CE);}

#define MISOout {SPI_DDR |= _BV(SPI_MISO);}
#define MISOin {SPI_DDR &= ~_BV(SPI_MISO);}

#define SS (SPI_PIN & _BV(SPI_SS))
#define CE (SPI_PIN & _BV(SPI_CE)) 

#define LEDon {PORTC |= _BV(PC0); DDRC |= _BV(PC0); }
#define LEDoff {PORTC &= ~_BV(PC0); DDRC &= ~_BV(PC0); }

#define E_NOTREADY 0
#define E_READY 0x02

// start condition is CE going from high to low and SS being low
#define START { \
	SPI_DDR |= (_BV(SPI_SS) | _BV(SPI_CE)); \
	SPI_PORT &= ~(_BV(SPI_SS) | _BV(SPI_CE)); \
}

#define EXT_RD (0x20)
#define EXT_WR (0x30)


enum states {
	BUS_IDLE,
	BUS_ADDRESS,
	BUS_COMMAND,
	BUS_RX_DATA,
	BUS_TX_DATA,
	BUS_CHECKSUM,
	BUS_FINISH,
	BUS_STATE_COUNT
};

enum events {
	EV_BUS_START,
	EV_BUS_END,
	EV_BUS_DATA,
	EV_COUNT
};

typedef struct bus_s {
	uint8_t rx_bytes;
	uint16_t address;
	uint8_t state;
	uint8_t command;
	uint8_t command_size; 
	uint8_t out_byte;
	uint8_t in_byte;
	uint8_t sum; 
} bus_t;

bus_t _bus = {
	.rx_bytes = 0,
	.address = 0,
	.state = BUS_IDLE, 
	.command = 0
}; 

static void _bus_reset(void);

// used by slave
static volatile char *_slave_buffer;
static volatile uint8_t _slave_buffer_size = 0; 
static volatile uint8_t _slave_buffer_ptr;
static volatile uint16_t _slave_addr = 0;

//static packet_t _packet;
//static volatile uint8_t _rx_count = 0; 
static volatile uint8_t _prev_pinb = 0;

void bus_master_init(void){
	SPI_DDR &= ~((1<<SPI_MISO)); //input
	SPI_DDR |= (_BV(SPI_MOSI) | _BV(SPI_SS) | _BV(SPI_SCK) | _BV(SPI_CE)); //output
	// enable pullups
	SPI_PORT |= (_BV(SPI_SS) | _BV(SPI_CE));
	
	SPCR = ((1<<SPE)|               // SPI Enable
					(0<<SPIE)|              // SPI Interupt Enable
					(0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
					(1<<MSTR)|              // Master/Slave select
					(0<<SPR1)|(0<<SPR0)|    // SPI Clock Rate
					(0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
					(0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

	SPSR = (0<<SPI2X); // Double SPI Speed Bit
	
	// default is SS high and CE high
	SPI_PORT |= _BV(SPI_CE) | _BV(SPI_SS); 
}

/** initialize spi interface in SPI_EXT mode **/
void bus_slave_init(uint16_t base_addr, char *buf, uint8_t size){
	//_slave_addr = base_addr & 0x7fff;
	_slave_addr = base_addr;
	_slave_buffer = buf;
	_slave_buffer_size = size;
	
	// Enable PCINT interrupt on PB0 (int0) and PB1 (int1) 
	PCICR |= (1<<PCIE0);  
	PCMSK0 = (1<<PCINT0) | (1<<PCINT2);

	SPI_DDR &= ~(_BV(SPI_MOSI) | _BV(SPI_SS) | _BV(SPI_CE) | _BV(SPI_SCK)); //input
	// enable pullups on CE and SS
	SPI_PORT |= (_BV(SPI_CE) | _BV(SPI_SS));
	
	// MISO is also input by default so that we don't interfere with other slaves. 
	MISOin; 
	
	SPCR = ((1<<SPE)|               // SPI Enable
					(1<<SPIE)|              // SPI Interupt Enable
					(0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
					(0<<MSTR)|              // Master/Slave select
					(0<<SPR1)|(0<<SPR0)|    // SPI Clock Rate
					(0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
					(0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

	SPSR = (0<<SPI2X); // Double SPI Speed Bit
}

uint8_t bus_exchange_byte(uint8_t data) {
    SPDR = data;
    while((SPSR & (1<<SPIF)) == 0);
    return SPDR;
}

uint16_t ext_get_address(void){ return _bus.address; }
uint16_t ext_state(void){ return 0;}



#define BYTE_DELAY_US 5

// writes data to an expansion slot peripheral
uint8_t bus_write(uint16_t addr, const char *data, uint8_t size){
	//if(!ext_start(addr & 0x7fff)) return 0; 
	if(size > 0x0f) size = 0x0f;
	
	SSout;
	SSlo;
	_delay_us(10);

	bus_exchange_byte(addr);
	_delay_us(5);
	
	bus_exchange_byte(addr >> 8);
	_delay_us(5);

	bus_exchange_byte(EXT_WR | (size & 0x0f));
	_delay_us(5);
	
	//uint8_t stat = bus_exchange_byte(0);
	//_delay_us(BYTE_DELAY_US);
		
	// transfer data
	uint8_t c = 0; 
	for(c = 0; c < size; c++){
		//SSlo;
		bus_exchange_byte(data[c]);
		//SShi;
		_delay_us(10);
	}
	
	SShi;
	_delay_us(10);

	return c; 
}
// reads data from expansion slot peripheral
uint8_t bus_read(uint16_t addr, char *data, uint8_t size){
	uint8_t retry = 10;
	uint8_t rx_count = 0; 
	while(retry--){
		cli();

		rx_count = 0;
			
		SSlo;
		_delay_us(BYTE_DELAY_US);

		bus_exchange_byte(addr);
		_delay_us(BYTE_DELAY_US);

		bus_exchange_byte(addr >> 8);
		_delay_us(BYTE_DELAY_US);

		bus_exchange_byte(EXT_RD | (size & 0x0f));
		_delay_us(BYTE_DELAY_US);

		// when slave gets the last address byte, it loads it's data register with previous
		// "next" byte. When it gets the command, it loads it with our status. 
		uint8_t stat = bus_exchange_byte(0);
		_delay_us(BYTE_DELAY_US);

		if(stat != E_READY){
			SShi;
			
			sei();
			
			_delay_us(50); 
			continue; 
		}
		
		//bus_exchange_byte(0);
		__asm("nop"); __asm("nop"); __asm("nop");
		
		// transfer data
		uint8_t sum = 0;
		uint8_t buffer[16];
		
		while(rx_count < size){
			buffer[rx_count] = bus_exchange_byte(0);
			sum += buffer[rx_count]; 
			rx_count++; 
			_delay_us(BYTE_DELAY_US); 
		}

		uint8_t sum2 = bus_exchange_byte(0);
		if(sum != sum2) {
			SShi;
			sei(); 
			_delay_us(2);
			continue;
		}

		for(uint8_t c = 0; c < 16; c++){
			data[c] = buffer[c];
		}
		
		break; 
	}
	_delay_us(BYTE_DELAY_US);
	
	SShi;
	sei();
	
	return rx_count; 
}


static void _bus_idle_start(void){
	// called when ss goes low
}

static void _bus_idle_rx_byte(void){
	_bus.address |= _bus.in_byte;
	_bus.sum = 0; 
	_bus.state = BUS_ADDRESS; 
}

static void _bus_address_rx_byte(void){
	_bus.address |= _bus.in_byte << 8;
	if((_bus.address >= _slave_addr) &&
		(_bus.address < (_slave_addr + _slave_buffer_size))){
		// if the address matches our address then we enter command state
		MISOout;
		_bus.state = BUS_COMMAND;
	} else {
		_bus_reset(); 
	}
}

static void _bus_command_rx_byte(void){
	_bus.command = _bus.in_byte & 0xf0;
	_bus.command_size = _bus.in_byte & 0x0f;
	if(_bus.command == EXT_RD)
		_bus.state = BUS_TX_DATA;
	else if(_bus.command == EXT_WR)
		_bus.state = BUS_RX_DATA;
		
	_bus.out_byte = E_READY;
	
	_slave_buffer_ptr = 0; 
}

static void _bus_rx_data(void){
	if(_slave_buffer_ptr < _slave_buffer_size){
		_slave_buffer[_slave_buffer_ptr++] = _bus.in_byte;
		_bus.out_byte = _bus.in_byte; // send it back to master for inspection
	} else {
		_bus_reset();
	}
}

static void _bus_checksum_rx_byte(void){
	_bus.out_byte = _bus.sum;
	_bus.state = BUS_FINISH;  
}

static void _bus_tx_data(void){
	if(_slave_buffer_ptr < _slave_buffer_size){
		_bus.out_byte = _slave_buffer[_slave_buffer_ptr++];
		_bus.sum += _bus.out_byte;
		//_bus.out_byte = _bus.sum;
		
		if(_slave_buffer_ptr == _bus.command_size){
			_bus.state = BUS_CHECKSUM;
		} 
	} else {
		_bus.state = BUS_FINISH; 
		//_bus_reset();
	}
	
	
}

static void _bus_reset(void){
	_bus.address = 0;
	_bus.command = 0;
	_bus.sum = 0; 
	_bus.state = BUS_IDLE;
	MISOin; 
}

static void _bus_nop(void){
	// does nothing :) 
}

// Bus state machine state/events callback table
// start, end, data
void (*const state_table [BUS_STATE_COUNT][EV_COUNT]) (void) = {
	{ _bus_idle_start, 	_bus_reset, _bus_idle_rx_byte,  }, // idle
	{ _bus_nop, 				_bus_reset, _bus_address_rx_byte }, // address
	{ _bus_nop, 				_bus_reset, _bus_command_rx_byte }, // command
	{ _bus_nop, 				_bus_reset, _bus_rx_data }, // rx
	{ _bus_nop, 				_bus_reset, _bus_tx_data }, // tx
	{ _bus_nop, 				_bus_reset, _bus_checksum_rx_byte}, // checksum
	{ _bus_nop, 				_bus_reset, _bus_reset} // finish
};

/// Enabled for the bus transfers
/*
ISR(PCINT0_vect){
	//DDRB |= _BV(1);
	
	uint8_t changed = _prev_pinb ^ SPI_PIN; 
	_prev_pinb = SPI_PIN;

	if(SPI_is_slave && (changed & _BV(SPI_SS))){
		if(!SS){
			state_table[_bus.state][EV_BUS_START]();
			//_bus.sync_timeout = timeout(5); 
		} else {
			//if(!timeout_expired(_bus.timeout)){
				// sync bus arbitration
			//} 
			state_table[_bus.state][EV_BUS_END]();
		}
	}
}
*/
/// This ISR is only used when data is received in slave mode.
/// Takes 1.5us
/// Interrupt latency: 2us
/// SPI transmission time: 4us at clk/8
ISR(SPI_STC_vect)
{
	//PORTB |= _BV(1);
	//DDRB |= _BV(1);
	
	_bus.in_byte = SPDR; 
	state_table[_bus.state][EV_BUS_DATA]();
	SPDR = _bus.out_byte;

	//PORTB &= ~_BV(1); 
}
