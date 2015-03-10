


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

