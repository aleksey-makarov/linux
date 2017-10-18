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
#include <linux/pci.h>

#define DRV_NAME	"Cavium PTP Driver"

#define PCI_DEVICE_ID_CAVIUM_PTP	0xA00C
#define PCI_DEVICE_ID_CAVIUM_RST	0xA00E

#define PTP_CLOCK_CFG		0xF00ULL
#define  PTP_CLOCK_CFG_PTP_EN	(1 << 0)
#define PTP_CLOCK_LO		0xF08ULL
#define PTP_CLOCK_HI		0xF10ULL
#define PTP_CLOCK_COMP		0xF18ULL

#define RST_BOOT		0x1600

#define DEFAULT_SCLK_MUL	16

/*
 * The Cavium PTP can *only* be found in SoCs containing the ThunderX ARM64 CPU
 * implementation.  All accesses to the device registers on this platform are
 * implicitly strongly ordered with respect to memory accesses. So
 * writeq_relaxed() and readq_relaxed() are safe to use with no memory barriers
 * in this driver.  The readq()/writeq() functions add explicit ordering
 * operation which in this case are redundant, and only add overhead.
 */

static u64 ptp_cavium_reg_read(struct ptp_cavium_clock *clock, u64 offset)
{
	return readq_relaxed(clock->reg_base + offset);
}

static void ptp_cavium_reg_write(struct ptp_cavium_clock *clock, u64 offset,
				 u64 val)
{
	writeq_relaxed(val, clock->reg_base + offset);
}

/**
 * ptp_cavium_adjfreq() - Adjust ptp frequency
 * @ptp: PTP clock info
 * @ppb: how much to adjust by, in parts-per-billion
 */
static int ptp_cavium_adjfreq(struct ptp_clock_info *ptp_info, s32 ppb)
{
	struct ptp_cavium_clock *clock =
		container_of(ptp_info, struct ptp_cavium_clock, ptp_info);
	unsigned long flags;
	u64 comp;
	u64 adj;
	bool neg_adj = false;

	if (ppb < 0) {
		neg_adj = true;
		ppb = -ppb;
	}

	/*
	 * The hardware adds the clock compensation value to the PTP clock on
	 * every coprocessor clock cycle. Typical convention is that it
	 * represent number of nanosecond betwen each cycle. In this convention
	 * compensation value is in 64 bit fixed-point representation where
	 * upper 32 bits are number of nanoseconds and lower is fractions of
	 * nanosecond.
	 * The ppb represent the ratio in "parts per bilion" by which the
	 * compensation value should be corrected.
	 * To calculate new compenstation value we use 64bit fixed point
	 * arithmetic on following formula comp = tbase + tbase*ppb/1G where
	 * tbase is the basic compensation value calculated initialy in
	 * ptp_cavium_init() -> tbase = 1/Hz. Then we use endian independent
	 * structure definition to write data to PTP register
	 */
	comp = ((u64)1000000000ull << 32) / clock->clock_rate;
	adj = comp * ppb;
	adj = div_u64(adj, 1000000000ull);
	comp = neg_adj ? comp - adj : comp + adj;

	spin_lock_irqsave(&clock->spin_lock, flags);
	ptp_cavium_reg_write(clock, PTP_CLOCK_COMP, comp);
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

	return ptp_cavium_reg_read(clock, PTP_CLOCK_HI);
}

/**
 * ptp_cavium_get_sclk_mul() - Get SCLK multiplier from RST block
 */
static u64 ptp_cavium_get_sclk_mul(void)
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

static int ptp_cavium_probe(struct pci_dev *pdev,
			    const struct pci_device_id *ent)
{
	struct device *dev = &pdev->dev;
	struct ptp_cavium_clock *clock;
	struct cyclecounter *cc;
	u64 clock_cfg;
	u64 clock_comp;
	int err;

	clock = devm_kzalloc(dev, sizeof(*clock), GFP_KERNEL);
	if (!clock)
		return -ENOMEM;

	clock->pdev = pdev;
	pci_set_drvdata(pdev, clock);

	err = pci_enable_device(pdev);
	if (err) {
		dev_err(dev, "Failed to enable PCI device 0x%x\n", err);
		pci_set_drvdata(pdev, NULL);
		return err;
	}

	err = pci_request_regions(pdev, DRV_NAME);
	if (err) {
		dev_err(dev, "PCI request regions failed 0x%x\n", err);
		goto err_disable_device;
	}

	/* MAP configuration registers */
	clock->reg_base = ioremap(pci_resource_start(pdev, 0),
			pci_resource_len(pdev, 0));
	if (!clock->reg_base) {
		dev_err(dev, "Cannot map CSR memory space\n");
		err = -ENOMEM;
		goto err_release_regions;
	}

	cc = &clock->cycle_counter;

	cc->read = ptp_cavium_cc_read;
	cc->mask = CYCLECOUNTER_MASK(64);
	cc->mult = 1;
	cc->shift = 0;

	timecounter_init(&clock->time_counter, &clock->cycle_counter,
			 ktime_to_ns(ktime_get_real()));

	spin_lock_init(&clock->spin_lock);
	clock->ptp_info = (struct ptp_clock_info) {
		.owner		= THIS_MODULE,
		.name		= "ThunderX PTP",
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

	/* enable PTP HW module */
	clock_cfg = ptp_cavium_reg_read(clock, PTP_CLOCK_CFG);
	clock_cfg |= PTP_CLOCK_CFG_PTP_EN;
	ptp_cavium_reg_write(clock, PTP_CLOCK_CFG, clock_cfg);

	/* The hardware adds the clock compensation value to the PTP clock on
	 * every coprocessor clock cycle. Typical convention is tha it represent
	 * number of nanosecond betwen each cycle. In this convention
	 * Compensation value is in 64 bit fixed-point representation where
	 * upper 32 bits are number of nanoseconds and lower is fractions of
	 * nanosecond. To calculate it we use 64bit fixed point arithmetic on
	 * following formula comp = t = 1/Hz. Then we use endian independent
	 * structire definition to write data to PTP register */
	clock_comp = ((u64)1000000000ull << 32) / clock->clock_rate;
	ptp_cavium_reg_write(clock, PTP_CLOCK_COMP, clock_comp);

	clock->clock_rate = ptp_cavium_get_sclk_mul() * 50000000ull,

	/* register PTP clock in kernel */
	clock->ptp_clock = ptp_clock_register(&clock->ptp_info, dev);
	if (IS_ERR(clock->ptp_clock))
		goto error;

	return 0;

error:
	/* stop PTP HW module */
	clock_cfg = ptp_cavium_reg_read(clock, PTP_CLOCK_CFG);
	clock_cfg &= ~PTP_CLOCK_CFG_PTP_EN;
	ptp_cavium_reg_write(clock, PTP_CLOCK_CFG, clock_cfg);

	devm_kfree(dev, clock);
err_release_regions:
	pci_release_regions(pdev);
err_disable_device:
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);

	devm_kfree(dev, clock);
	return err;
}

static void ptp_cavium_remove(struct pci_dev *pdev)
{
	struct ptp_cavium_clock *clock = pci_get_drvdata(pdev);
	u64 clock_cfg;

	/* stop PTP HW module */
	clock_cfg = ptp_cavium_reg_read(clock, PTP_CLOCK_CFG);
	clock_cfg &= ~PTP_CLOCK_CFG_PTP_EN;
	ptp_cavium_reg_write(clock, PTP_CLOCK_CFG, clock_cfg);

	ptp_clock_unregister(clock->ptp_clock);

	iounmap(clock->reg_base);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);
}

static const struct pci_device_id ptp_cavium_id_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_CAVIUM, PCI_DEVICE_ID_CAVIUM_PTP) },
	{ 0, }
};

static struct pci_driver ptp_cavium_driver = {
	.name = DRV_NAME,
	.id_table = ptp_cavium_id_table,
	.probe = ptp_cavium_probe,
	.remove = ptp_cavium_remove,
};

static int __init ptp_cavium_init_module(void)
{
	return pci_register_driver(&ptp_cavium_driver);
}

static void __exit ptp_cavium_cleanup_module(void)
{
	pci_unregister_driver(&ptp_cavium_driver);
}

module_init(ptp_cavium_init_module);
module_exit(ptp_cavium_cleanup_module);

MODULE_DESCRIPTION(DRV_NAME);
MODULE_AUTHOR("Cavium Networks <support@cavium.com>");
MODULE_LICENSE("GPL v2");
