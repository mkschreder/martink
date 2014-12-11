CPU support
===========

| VENDOR   |    Model      |Status |
|----------|:-------------:|------:|
|   ATMEL  |  ATMega328p   |  yes  |
|   ATMEL  |   AT91SAM3    |  yes  |
|   ST     |   STM32F103   |  part |

Device driver support
=====================

Display drivers
----------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| ILI9340  |  TFT display   |  yes  |
| LCDPFF8574   |   PCF8574 based lcd displays    |  yes  |
| LEDMATRIX | Led matrix display | not tested |
| SEVSEG | seven segment display | not tested | 
| SSD1306 | monochrome OLED display | yes | 

Filesystem drivers
------------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| FAT  |  Fat filesystem   |  not ported  |

HID
---

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| WIINUNCHUCK  |  Wii nunchuck   |  not tested  |

IO peripheral
-------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| L74HC165  |  Shift in register   |  yes  |
| L74HC4051   |   Multiplexer    |  yes  |
| L74HC595 | Shift out register | yes |
| PCF8574 | I2C GPIO expander | yes | 

Motor servo drivers
-------------------

These drivers need to be ported to use hardware or software pwm devices. 

A lot of these should support driving motors not just over gpio and pwm, but also over io expanders. This reaquires needs generic gpio access that will be implemented in arch support. 

Hardware PWM will be implemented in arch support code

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| BLDC  |  Brushless dc motor   |  not ported  |
| BLDCSL   |   Brushless sensorless    |  not ported  |
| DCMOTOR | DC motor pwm | not ported |
| SEVSEG | seven segment display | not tested | 
| PWM | Hardware pwm access | not ported | 
| STEPPER | Stepper motor support | not ported | 

Network adapters
----------------

These need to be modified to use arbitrary ethernet or packet interface. It should not matter if we use ENC28 IC or NRF module. It should still be the same code. Need generic network interface. 

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| ENC28J60  |  Ethernet driver IC   |  not tested  |
| TCPIP   | Minimal IP stack    |  not tested  |
| RFNET | Encrypted radio com | yes |

Radio module support
--------------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| NRF24L01  |  Radio module  |  not tested  |

RFID support
------------

These need to be ported and also need to use generic interface for access control input. Probably even as a hid module. 

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| MFRC522  |  13mhz rfid card reader   |  not ported  |

Sensor drivers
--------------

Many working sensors. Others need to be tested and ported to the portable api. 

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| TODO  |  todo   |  todo  |

TTY
----------

TTY drivers need to use generic display interface so that it can be used with any GLCD display. 

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| VT100  |  VT100 emulator   |  yes  |
