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

	struct pci_dev *pdev;
	void __iomem *reg_base;

	spinlock_t spin_lock;
	struct cyclecounter cycle_counter;
	struct timecounter time_counter;
	u32 clock_rate;

	struct ptp_clock_info ptp_info;
	struct ptp_clock *ptp_clock;
};

#endif
