/*
 * ptp_cavium.c - PTP 1588 clock on Cavium hardware
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

#include <linux/device.h>
#include <linux/module.h>
#include <linux/timecounter.h>
#include <linux/ptp_cavium.h>

#define PTP_CLOCK_CFG		0xF00ULL
#define  PTP_CLOCK_CFG_PTP_EN	(1 << 0)
#define PTP_CLOCK_LO		0xF08ULL
#define PTP_CLOCK_HI		0xF10ULL
#define PTP_CLOCK_COMP		0xF18ULL

/**
 * ptp_cavium_adjfreq() - Adjust ptp frequency
 * @ptp: PTP clock info
 * @ppb: how much to adjust by, in parts-per-billion
 */
static int ptp_cavium_adjfreq(struct ptp_clock_info *ptp_info, s32 ppb)
{
	struct ptp_cavium_clock *clock =
		container_of(ptp_info, struct ptp_cavium_clock, ptp_info);
	struct ptp_cavium_clock_info *info = clock->ptp_cavium_info;
	unsigned long flags;
	u64 comp;
	u64 adj;
	bool neg_adj = false;

	if (ppb < 0) {
		neg_adj = true;
		ppb = -ppb;
	}

	/* The hardware adds the clock compensation value to the PTP clock on
	 * every coprocessor clock cycle. Typical convention is that it
	 * represent number of nanosecond betwen each cycle. In this convention
	 * Compensation value is in 64 bit fixed-point representation where
	 * upper 32 bits are number of nanoseconds and lower is fractions of
	 * nanosecond.
	 * The ppb represent the ratio in "parts per bilion" by which the
	 * compensation value should be corrected.
	 * To calculate new compenstation value we use 64bit fixed point
	 * arithmetic on following formula comp = tbase + tbase*ppb/1G where
	 * tbase is the basic compensation value calculated initialy in
	 * ptp_cavium_init() -> tbase = 1/Hz. Then we use endian independent
	 * structure definition to write data to PTP register */
	comp = ((u64)1000000000ull << 32) / info->clock_rate;
	adj = comp * ppb;
	adj = div_u64(adj, 1000000000ull);
	comp = neg_adj ? comp - adj : comp + adj;

	spin_lock_irqsave(&clock->spin_lock, flags);
	info->reg_write(info, PTP_CLOCK_COMP, comp);
	spin_unlock_irqrestore(&clock->spin_lock, flags);

	return 0;
}

/**
 * ptp_cavium_adjtime() - Adjust ptp time
 * @ptp:   PTP clock info
 * @delta: how much to adjust by, in nanosecs
 */
static int ptp_cavium_adjtime(struct ptp_clock_info *ptp_info, s64 delta)
{
	struct ptp_cavium_clock *clock =
		container_of(ptp_info, struct ptp_cavium_clock, ptp_info);
	unsigned long flags;

	spin_lock_irqsave(&clock->spin_lock, flags);
	timecounter_adjtime(&clock->time_counter, delta);
	spin_unlock_irqrestore(&clock->spin_lock, flags);

	/* Sync, for network driver to get latest value */
	smp_mb();

	return 0;
}

/**
 * ptp_cavium_gettime() - Get hardware clock time, including any adjustment
 * @ptp: PTP clock info
 * @ts:  timespec
 */
static int ptp_cavium_gettime(struct ptp_clock_info *ptp_info,
			      struct timespec64 *ts)
{
	struct ptp_cavium_clock *clock =
		container_of(ptp_info, struct ptp_cavium_clock, ptp_info);
	unsigned long flags;
	u64 nsec;

	spin_lock_irqsave(&clock->spin_lock, flags);
	nsec = timecounter_read(&clock->time_counter);
	spin_unlock_irqrestore(&clock->spin_lock, flags);

	*ts = ns_to_timespec64(nsec);

	return 0;
}

/**
 * ptp_cavium_settime() - Set hardware clock time. Reset adjustment
 * @ptp: PTP clock info
 * @ts:  timespec
 */
static int ptp_cavium_settime(struct ptp_clock_info *ptp_info,
			      const struct timespec64 *ts)
{
	struct ptp_cavium_clock *clock =
		container_of(ptp_info, struct ptp_cavium_clock, ptp_info);
	unsigned long flags;
	u64 nsec;

	nsec = timespec64_to_ns(ts);

	spin_lock_irqsave(&clock->spin_lock, flags);
	timecounter_init(&clock->time_counter, &clock->cycle_counter, nsec);
	spin_unlock_irqrestore(&clock->spin_lock, flags);

	return 0;
}

/**
 * ptp_cavium_enable() - Check if PTP is enabled
 * @ptp: PTP clock info
 * @rq:  request
 * @on:  is it on
 */
static int ptp_cavium_enable(struct ptp_clock_info *ptp_info,
			     struct ptp_clock_request *rq, int on)
{
	return -EOPNOTSUPP;
}

static u64 ptp_cavium_cc_read(const struct cyclecounter *cc)
{
	struct ptp_cavium_clock *clock =
		container_of(cc, struct ptp_cavium_clock, cycle_counter);
	struct ptp_cavium_clock_info *info = clock->ptp_cavium_info;

	return info->reg_read(info, PTP_CLOCK_HI);
}

struct ptp_cavium_clock* ptp_cavium_register(struct ptp_cavium_clock_info *info,
					     struct device *dev)
{
	struct ptp_cavium_clock *clock = NULL;
	struct cyclecounter *cc;
	u64 clock_cfg;
	u64 clock_comp;

	clock = devm_kzalloc(dev, sizeof(*clock), GFP_KERNEL);
	if (!clock)
		return NULL;

	cc = &clock->cycle_counter;

	cc->read = ptp_cavium_cc_read;
	cc->mask = CYCLECOUNTER_MASK(64);
	cc->mult = 1;
	cc->shift = 0;

	timecounter_init(&clock->time_counter, &clock->cycle_counter,
			 ktime_to_ns(ktime_get_real()));

	clock->ptp_cavium_info = info;
	spin_lock_init(&clock->spin_lock);
	clock->ptp_info = (struct ptp_clock_info) {
		.owner		= THIS_MODULE,
		.max_adj	= 1000000000ull,
		.n_ext_ts	= 0,
		.n_pins		= 0,
		.pps		= 0,
		.adjfreq	= ptp_cavium_adjfreq,
		.adjtime	= ptp_cavium_adjtime,
		.gettime64	= ptp_cavium_gettime,
		.settime64	= ptp_cavium_settime,
		.enable		= ptp_cavium_enable,
	};
	snprintf(clock->ptp_info.name, 16, "%s", info->name);

	/* enable PTP HW module */
	clock_cfg = info->reg_read(info, PTP_CLOCK_CFG);
	clock_cfg |= PTP_CLOCK_CFG_PTP_EN;
	info->reg_write(info, PTP_CLOCK_CFG, clock_cfg);

	/* The hardware adds the clock compensation value to the PTP clock on
	 * every coprocessor clock cycle. Typical convention is tha it represent
	 * number of nanosecond betwen each cycle. In this convention
	 * Compensation value is in 64 bit fixed-point representation where
	 * upper 32 bits are number of nanoseconds and lower is fractions of
	 * nanosecond. To calculate it we use 64bit fixed point arithmetic on
	 * following formula comp = t = 1/Hz. Then we use endian independent
	 * structire definition to write data to PTP register */
	clock_comp = ((u64)1000000000ull << 32) / info->clock_rate;
	info->reg_write(info, PTP_CLOCK_COMP, clock_comp);

	/* register PTP clock in kernel */
	clock->ptp_clock = ptp_clock_register(&clock->ptp_info, dev);
	if (IS_ERR(clock->ptp_clock))
		goto error;

	return clock;

error:
	/* stop PTP HW module */
	clock_cfg = info->reg_read(info, PTP_CLOCK_CFG);
	clock_cfg &= ~PTP_CLOCK_CFG_PTP_EN;
	info->reg_write(info, PTP_CLOCK_CFG, clock_cfg);

	devm_kfree(dev, clock);
	return NULL;
}
EXPORT_SYMBOL(ptp_cavium_register);

void ptp_cavium_unregister(struct ptp_cavium_clock *clock)
{
	u64 clock_cfg;
	struct ptp_cavium_clock_info *info = clock->ptp_cavium_info;

	/* stop PTP HW module */
	clock_cfg = info->reg_read(info, PTP_CLOCK_CFG);
	clock_cfg &= ~PTP_CLOCK_CFG_PTP_EN;
	info->reg_write(info, PTP_CLOCK_CFG, clock_cfg);

	ptp_clock_unregister(clock->ptp_clock);
}
EXPORT_SYMBOL(ptp_cavium_unregister);

static int __init ptp_cavium_init_module(void)
{
	return 0;
}

static void __exit ptp_cavium_cleanup_module(void)
{
}

module_init(ptp_cavium_init_module);
module_exit(ptp_cavium_cleanup_module);

MODULE_DESCRIPTION("Cavium generic PTP Driver");
MODULE_AUTHOR("Cavium Networks <support@cavium.com>");
MODULE_LICENSE("GPL v2");
/**********************************************************************
* Author: Cavium, Inc.
*
* Contact: support@cavium.com
*          Please include "LiquidIO" in the subject.
*
* Copyright (c) 2003-2015 Cavium, Inc.
*
* This file is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, Version 2, as
* published by the Free Software Foundation.
*
* This file is distributed in the hope that it will be useful, but
* AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
* NONINFRINGEMENT.  See the GNU General Public License for more
* details.
*
* This file may also be available under a different license from Cavium.
* Contact Cavium, Inc. for more information
**********************************************************************/

#include <linux/device.h>
#include <linux/module.h>
#include <linux/pci.h>

#include "cavium_ptp.h"

#define NSEC_PER_SEC     1000000000L
#define DRV_NAME         "Cavium Thunder PTP Driver"
#define DRV_VERSION      "1.0"

/* PCI device IDs */
#define PCI_DEVICE_ID_THUNDER_PTP	0xA00C

struct thunder_ptp_clock *thunder_ptp_clock;
EXPORT_SYMBOL(thunder_ptp_clock);

/*
 * Register access functions
 */

/* The Cavium PTP can *only* be found in SoCs containing the ThunderX ARM64 CPU
 * implementation.  All accesses to the device registers on this platform are
 * implicitly strongly ordered with respect to memory accesses. So
 * writeq_relaxed() and readq_relaxed() are safe to use with no memory barriers
 * in this driver.  The readq()/writeq() functions add explicit ordering
 * operation which in this case are redundant, and only add overhead.
 */

static u64 thunder_ptp_reg_read(struct cavium_ptp_clock_info *info,
				    u64 offset)
{
	struct thunder_ptp_clock *thunder_ptp_clock =
		container_of(info, struct thunder_ptp_clock, cavium_ptp_info);
	void __iomem *addr = thunder_ptp_clock->reg_base + offset;

	return readq_relaxed(addr);
}

static void thunder_ptp_reg_write(struct cavium_ptp_clock_info *info,
				      u64 offset, u64 val)
{
	struct thunder_ptp_clock *thunder_ptp_clock =
		container_of(info, struct thunder_ptp_clock, cavium_ptp_info);
	void __iomem *addr = thunder_ptp_clock->reg_base + offset;

	writeq_relaxed(val, addr);
}

static void thunder_ptp_adjtime(struct cavium_ptp_clock_info *info,
				   s64 delta)
{
	struct thunder_ptp_clock *thunder_ptp_clock =
		container_of(info, struct thunder_ptp_clock, cavium_ptp_info);

	thunder_ptp_clock->ptp_adjust = delta;
}

s64 thunder_get_adjtime(void)
{
	if (!thunder_ptp_clock)
		return 0;

	return thunder_ptp_clock->ptp_adjust;
}
EXPORT_SYMBOL(thunder_get_adjtime);

#define PCI_DEVICE_ID_CAVIUM_RST 0xA00E
#define DEFAULT_SCLK_MUL	 16
#define RST_BOOT		 0x1600

/* Get SCLK multiplier from RST block */
static u64 thunder_get_sclk_mul(void)
{
	struct pci_dev *rstdev;
	void __iomem *rst_base = NULL;
	u64 sclk_mul = DEFAULT_SCLK_MUL;

	rstdev = pci_get_device(PCI_VENDOR_ID_CAVIUM,
				PCI_DEVICE_ID_CAVIUM_RST, NULL);
	if (!rstdev)
		return sclk_mul;

	rst_base = ioremap(pci_resource_start(rstdev, 0),
			   pci_resource_len(rstdev, 0));
	if (rst_base) {
		sclk_mul = readq_relaxed(rst_base + RST_BOOT);
		sclk_mul = (sclk_mul >> 33) & 0x3F;
		iounmap(rst_base);
	}

	return sclk_mul;
}

/* module operations */

static int thunder_ptp_probe(struct pci_dev *pdev,
				 const struct pci_device_id *ent)
{
	int err;
	struct device *dev = &pdev->dev;

	thunder_ptp_clock = devm_kzalloc(dev, sizeof(*thunder_ptp_clock),
					 GFP_KERNEL);
	if (!thunder_ptp_clock)
		return -ENOMEM;
	thunder_ptp_clock->pdev = pdev;
	pci_set_drvdata(pdev, thunder_ptp_clock);

	err = pci_enable_device(pdev);
	if (err) {
		dev_err(dev, "Failed to enable PCI device\n");
		pci_set_drvdata(pdev, NULL);
		return err;
	}

	err = pci_request_regions(pdev, DRV_NAME);
	if (err) {
		dev_err(dev, "PCI request regions failed 0x%x\n", err);
		goto err_disable_device;
	}

	/* MAP configuration registers */
	thunder_ptp_clock->reg_base = ioremap(pci_resource_start(pdev, 0),
					    pci_resource_len(pdev, 0));
	if (!thunder_ptp_clock->reg_base) {
		dev_err(dev, "BGX: Cannot map CSR memory space, aborting\n");
		err = -ENOMEM;
		goto err_release_regions;
	}

	/* register the cavium_ptp_clock */
	thunder_ptp_clock->cavium_ptp_info = (struct cavium_ptp_clock_info) {
		/* Below we need to give the SCLK which is:
		 * PLL_REF_CLK (= 50 MHz) × [PNR_MUL]
		 */
		.clock_rate = thunder_get_sclk_mul() * 50000000ull,
		.name = "ThunderX PTP",
		.reg_read = thunder_ptp_reg_read,
		.reg_write = thunder_ptp_reg_write,
		.adjtime_clbck = thunder_ptp_adjtime,
	};
	thunder_ptp_clock->cavium_ptp_clock = cavium_ptp_register(
		&thunder_ptp_clock->cavium_ptp_info, dev);
	if (IS_ERR(thunder_ptp_clock->cavium_ptp_clock))
		goto err_release_regions;

	return 0;

err_release_regions:
	pci_release_regions(pdev);
err_disable_device:
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);

	devm_kfree(dev, thunder_ptp_clock);
	return err;
}

static void thunder_ptp_remove(struct pci_dev *pdev)
{
	struct thunder_ptp_clock *thunder_ptp_clock = pci_get_drvdata(pdev);

	cavium_ptp_remove(thunder_ptp_clock->cavium_ptp_clock);
	iounmap(thunder_ptp_clock->reg_base);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);
}

/* Supported devices */
static const struct pci_device_id thunder_ptp_id_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_CAVIUM, PCI_DEVICE_ID_THUNDER_PTP) },
	{ 0, }  /* end of table */
};

static struct pci_driver thunder_ptp_driver = {
	.name = DRV_NAME,
	.id_table = thunder_ptp_id_table,
	.probe = thunder_ptp_probe,
	.remove = thunder_ptp_remove,
};

static int __init thunder_ptp_init_module(void)
{
	pr_info("%s, ver %s\n", DRV_NAME, DRV_VERSION);

	return pci_register_driver(&thunder_ptp_driver);
}

static void __exit thunder_ptp_cleanup_module(void)
{
	pci_unregister_driver(&thunder_ptp_driver);
}

module_init(thunder_ptp_init_module);
module_exit(thunder_ptp_cleanup_module);

MODULE_AUTHOR("Cavium Networks, <support@cavium.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(DRV_NAME);
MODULE_VERSION(DRV_VERSION);
MODULE_DEVICE_TABLE(pci, thunder_ptp_id_table);
