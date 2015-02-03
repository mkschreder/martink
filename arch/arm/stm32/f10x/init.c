/*
 * init.c
 *
 *  Created on: 27.06.2012
 *      Author: ryabinin
 */

extern int __init_array_start;
extern int __init_array_end;

void ConstructorsInit(void)
{
	int *s, *e;

	// call the constructorts of global objects
	s = &__init_array_start;
	e = &__init_array_end;
	while (s != e)
	{
		(*((void (**)(void)) s++))();
	}
}

extern int main(void); 
extern void SystemInit(void); 

void c_startup(void)
{
	char *src, *dst;
	extern char _etext, _sdata, _sbss, _ebss, _edata; 
	
	src = &_etext;
	dst = &_sdata;
	while(dst < &_edata) 
		*(dst++) = *(src++);

	src = &_sbss;
	while(src < &_ebss) 
		*(src++) = 0;
	
	SystemInit(); 
	
	ConstructorsInit(); 
	
	main();
}
