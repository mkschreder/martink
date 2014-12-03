/*
dcmotorpwm lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef DCMOTORPWM_H
#define DCMOTORPWM_H

//set motor port
#define DCMOTORPWM_DDR DDRB
#define DCMOTORPWM_PORT PORTB
#define DCMOTORPWM_PIN1 PB1
#define DCMOTORPWM_PIN2 PB2

//set minimum velocity
#define DCMOTORPWM_MINVEL 180

//freq = 1 / time
//pulse freq = FCPU / prescaler * ICR1
// 125 = 1000000 / (8 * 1000)
#define DCMOTORPWM_ICR1 1000
#define DCMOTORPWM_PRESCALER (1 << CS11)

//functions
extern void dcmotorpwm_init(void);
extern void dcmotorpwm_gostop(void);
void dcmotorpwm_goforward(uint8_t vel);
void dcmotorpwm_gobackward(uint8_t vel);

#endif
