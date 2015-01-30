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
