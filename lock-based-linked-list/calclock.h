#ifndef __CALCLOCK_H
#define __CALCLOCK_H

#include <linux/time.h>

#define BILLION 1000000000UL

unsigned long long calclock(struct timespec *myclock, 
		unsigned long long *total_time, unsigned long long *total_count);

#endif /* __CALCLOCK_H */

