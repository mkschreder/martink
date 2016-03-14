#pragma once

#define module_param(a, b, c) 
#define MODULE_PARM_DESC(a, b)
#define MODULE_DEVICE_TABLE(a, b)

#undef __init
#undef __exit
#define __init __attribute__ ((constructor))
#define __exit __attribute__ ((destructor))


