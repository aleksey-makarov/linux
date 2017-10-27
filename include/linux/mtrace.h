#ifndef __mtrace_h__
#define __mtrace_h__

#include <linux/kernel.h>
#include <linux/stringify.h>

#define MTRACE_WHERE "-- " __FILE__ ":" __stringify(__LINE__) " (%s) : "

#define MTRACE( format, ... ) printk(KERN_WARNING MTRACE_WHERE format "\n" , __FUNCTION__, ##__VA_ARGS__ )

#endif
