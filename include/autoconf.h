/*
 * Automatically generated C config: don't edit
 * version: 
 */
#define AUTOCONF_TIMESTAMP "2014-12-22 01:54:56 CET"


/*
 * Hardware
 */
#define CONFIG_HAVE_ADC 1
#define CONFIG_HAVE_UART 1
#define CONFIG_HAVE_UART0 1
#define CONFIG_HAVE_SPI 1
#define CONFIG_HAVE_SPI0 1
#define CONFIG_HAVE_TWI 1
#define CONFIG_HAVE_TWI0 1
#define CONFIG_HAVE_TIMER0 1
#define CONFIG_HAVE_TIMER1 1
#define CONFIG_HAVE_TIMER2 1
#define CONFIG_AVR 1

/*
 * Processor type
 */
#define CONFIG_ATMEGA328P 1

/*
 * Configure ATMega328P driver
 */

/*
 * ATmega328P general options
 */
#define CONFIG_TIMESTAMP_COUNTER 1
#define CONFIG_GPIO_PIN_STATES 1

/*
 * UART Config ---
 */
#define CONFIG_BUFFERED_UART 1
#define CONFIG_UART0_TX_BUF_SIZE 32
#define CONFIG_UART0_RX_BUF_SIZE 32
#define CONFIG_UART0_NAME uart0
#define CONFIG_UART1_NAME 
#define CONFIG_UART2_NAME 

/*
 * SPI config ---
 */
#define CONFIG_SPI0_NAME spi0
#define CONFIG_SPI1_NAME 
#define CONFIG_SPI2_NAME 

/*
 * I2C/TWI config ---
 */
#define CONFIG_TWI0_NAME twi0
#define CONFIG_TWI1_NAME 

/*
 * Board support
 */
#define CONFIG_BOARD_MULTIWII 1
#define CONFIG_MULTIWII_ACC mpu6050

/*
 * Device driver support
 */

/*
 * Cryptographic support
 */
#define CONFIG_CRYPTO 1
#define CONFIG_CRYPTO_AES 1

/*
 * Display support
 */
#define CONFIG_ILI9340 1
#define CONFIG_LCDPAR 1
#define CONFIG_LEDMATRIX88 1
#define CONFIG_SSD1306 1
#define CONFIG_SEVSEG 1

/*
 * HID support
 */
#define CONFIG_WIINUNCHUK 1

/*
 * IO logic support
 */
#define CONFIG_L74HC165 1
#define CONFIG_L74HC4051 1
#define CONFIG_L74HC595 1
#define CONFIG_PCF8574 1

/*
 * Network support
 */
#define CONFIG_ENC28J60 1
#define CONFIG_DHCP 1
#define CONFIG_DNS 1
#define CONFIG_TCPIP 1

/*
 * Radio modem support
 */
#define CONFIG_NRF24L01 1

/*
 * RFID readers support
 */
#define CONFIG_MFRC522 1

/*
 * Sensor drivers
 */
#define CONFIG_ACS712 1
#define CONFIG_ADXL345 1
#define CONFIG_AMT1001 1
#define CONFIG_BH1750 1
#define CONFIG_BMP085 1
#define CONFIG_DHT 1
#define CONFIG_DS18B20 1
#define CONFIG_FS300A 1
#define CONFIG_HCSR04 1
#define CONFIG_HMC5883L 1
#define CONFIG_IMU10DOF01 1
#define CONFIG_L3G4200D 1
#define CONFIG_MMA7455 1
#define CONFIG_MPU6050 1
#define CONFIG_TEMT6000 1
#define CONFIG_TSL235 1
#define CONFIG_VT100 1

/*
 * Application Configuration
 */
#define CONFIG_HARDWARE 1
