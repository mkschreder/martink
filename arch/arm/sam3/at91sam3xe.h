#pragma once 

extern void cpu_init(void); 
/*
#define GPIO_NONE					-1
#define GPIO_PA0          0
#define GPIO_PA1          1
#define GPIO_PA2          2
#define GPIO_PA3          3
#define GPIO_PA4          4
#define GPIO_PA5          5
#define GPIO_PA6          6
#define GPIO_PA7          7
#define GPIO_PA8          8
#define GPIO_PA9          9
#define GPIO_PA10         10
#define GPIO_PA11         11
#define GPIO_PA12         12
#define GPIO_PA13         13
#define GPIO_PA14         14
#define GPIO_PA15         15
#define GPIO_PA16         16
#define GPIO_PA17         17
#define GPIO_PA18         18
#define GPIO_PA19         19
#define GPIO_PA20         20
#define GPIO_PA21         21
#define GPIO_PA22         22
#define GPIO_PA23         23
#define GPIO_PA24         24
#define GPIO_PA25         25
#define GPIO_PA26         26
#define GPIO_PA27         27
#define GPIO_PA28         28
#define GPIO_PA29         29
#define GPIO_PB0          32
#define GPIO_PB1          33
#define GPIO_PB2          34
#define GPIO_PB3          35
#define GPIO_PB4          36
#define GPIO_PB5          37
#define GPIO_PB6          38
#define GPIO_PB7          39
#define GPIO_PB8          40
#define GPIO_PB9          41
#define GPIO_PB10         42
#define GPIO_PB11         43
#define GPIO_PB12         44
#define GPIO_PB13         45
#define GPIO_PB14         46
#define GPIO_PB15         47
#define GPIO_PB16         48
#define GPIO_PB17         49
#define GPIO_PB18         50
#define GPIO_PB19         51
#define GPIO_PB20         52
#define GPIO_PB21         53
#define GPIO_PB22         54
#define GPIO_PB23         55
#define GPIO_PB24         56
#define GPIO_PB25         57
#define GPIO_PB26         58
#define GPIO_PB27         59
#define GPIO_PB28         60
#define GPIO_PB29         61
#define GPIO_PB30         62
#define GPIO_PB31         63
#define GPIO_PC0          64
#define GPIO_PC1          65
#define GPIO_PC2          66
#define GPIO_PC3          67
#define GPIO_PC4          68
#define GPIO_PC5          69
#define GPIO_PC6          70
#define GPIO_PC7          71
#define GPIO_PC8          72
#define GPIO_PC9          73
#define GPIO_PC10         74
#define GPIO_PC11         75
#define GPIO_PC12         76
#define GPIO_PC13         77
#define GPIO_PC14         78
#define GPIO_PC15         79
#define GPIO_PC16         80
#define GPIO_PC17         81
#define GPIO_PC18         82
#define GPIO_PC19         83
#define GPIO_PC20         84
#define GPIO_PC21         85
#define GPIO_PC22         86
#define GPIO_PC23         87
#define GPIO_PC24         88
#define GPIO_PC25         89
#define GPIO_PC26         90
#define GPIO_PC27         91
#define GPIO_PC28         92
#define GPIO_PC29         93
#define GPIO_PC30         94
#define GPIO_PD0          96
#define GPIO_PD1          97
#define GPIO_PD2          98
#define GPIO_PD3          99
#define GPIO_PD4          100
#define GPIO_PD5          101
#define GPIO_PD6          102
#define GPIO_PD7          103
#define GPIO_PD8          104
#define GPIO_PD9          105
#define GPIO_PD10         106

typedef uint16_t gpio_pin_t; 

#define GPPIO(pin) ((pin >= 0 && pin <= 31)?PIOA:(\
	(pin >= 32 && pin <= 63)?PIOB:(\
	(pin >= 64 && pin <= 94)?PIOC:(\
	(pin >= 95 && pin <= 106)?PIOD:PIOD))))

#define GPMASK(pin) (1 << (pin & 0x1f))

#define gpio_set(pin) {GPPIO(pin)->PIO_SODR = GPMASK(pin);}
#define gpio_clear(pin) {GPPIO(pin)->PIO_CODR = GPMASK(pin);}
#define gpio_read(pin) ((GPPIO(pin)->PIO_ODSR & GPMASK(pin))?1:0)
*/
