/*
 * ptp_cavium.h - PTP 1588 clock on Cavium hardware
 *
 * Copyright (c) 2003-2015, 2017 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 */

#ifndef _LINUX_PTP_CAVIUM_H
#define _LINUX_PTP_CAVIUM_H

#include <linux/ptp_clock_kernel.h>

struct ptp_cavium_clock {

	struct cyclecounter cycle_counter;
	struct timecounter time_counter;

	/* PTP clock information */
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info ptp_info;

	/* descendant data */
	spinlock_t spin_lock;
	struct ptp_cavium_clock_info *ptp_cavium_info;
};

struct ptp_cavium_clock_info {
	u32 clock_rate;
	const char *name;
	u64 (*reg_read)(struct ptp_cavium_clock_info *info, u64 offset);
	void (*reg_write)(struct ptp_cavium_clock_info *info, u64 offset,
			  u64 val);
	void (*adjtime_clbck)(struct ptp_cavium_clock_info *info, s64 delta);
};

struct ptp_cavium_clock *ptp_cavium_register(struct ptp_cavium_clock_info *info,
					     struct device *dev);
void ptp_cavium_unregister(struct ptp_cavium_clock *ptp_cavium_clock);

#endif
