/*
 * cavium_rst.h - Rest and Fuses (RST) Cavium hardware
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
#include <linux/pci.h>

#define DRV_NAME	"Cavium RST Driver"

#define PCI_DEVICE_ID_CAVIUM_RST	0xA00E
#define PCI_RST_BAR_NO			0

#define RST_BOOT	0x1600ULL

#define CLOCK_BASE_RATE	50000000ULL

struct cavium_rst {
	struct pci_dev *pdev;
	void __iomem *base;
};

u64 cavium_rst_get_clock_rate(struct cavium_rst *rst)
{
	/* The Cavium RST can *only* be found in SoCs containing the ThunderX
	 * ARM64 CPU implementation.  All accesses to the device registers on
	 * this platform are implicitly strongly ordered with respect to memory
	 * accesses. So writeq_relaxed() and readq_relaxed() are safe to use
	 * with no memory barriers in this driver.  The readq()/writeq()
	 * functions add explicit ordering operation which in this case are
	 * redundant, and only add overhead.
	 */
	u64 sclk_mul = readq_relaxed(rst->base + RST_BOOT);

	return ((sclk_mul >> 33) & 0x3F) * CLOCK_BASE_RATE;
}
EXPORT_SYMBOL(cavium_rst_get_clock_rate);

const u64 cavium_rst_clock_rate_default = 16 * CLOCK_BASE_RATE;
EXPORT_SYMBOL(cavium_rst_clock_rate_default);

struct cavium_rst *cavium_rst_get(void)
{
	struct pci_dev *pdev;
	struct cavium_rst *rst;

	pdev = pci_get_device(PCI_VENDOR_ID_CAVIUM,
			      PCI_DEVICE_ID_CAVIUM_RST, NULL);
	if (!pdev)
		return ERR_PTR(-ENODEV);

	rst = pci_get_drvdata(pdev);
	if (!rst) {
		pci_dev_put(pdev);
		rst = ERR_PTR(-EPROBE_DEFER);
	}

	return rst;
}
EXPORT_SYMBOL(cavium_rst_get);

void cavium_rst_put(struct cavium_rst *rst)
{
	pci_dev_put(rst->pdev);
}
EXPORT_SYMBOL(cavium_rst_put);

static int cavium_rst_probe(struct pci_dev *pdev,
			    const struct pci_device_id *ent)
{
	struct device *dev = &pdev->dev;
	struct cavium_rst *rst;
	int err;

	rst = devm_kzalloc(dev, sizeof(*rst), GFP_KERNEL);
	if (!rst)
		return -ENOMEM;

	err = pcim_enable_device(pdev);
	if (err)
		return err;

	err = pcim_iomap_regions(pdev, 1 << PCI_RST_BAR_NO, pci_name(pdev));
	if (err)
		return err;

	rst->base = pcim_iomap_table(pdev)[PCI_RST_BAR_NO];
	rst->pdev = pdev;

	pci_set_drvdata(pdev, rst);

	return 0;
}

static void cavium_rst_remove(struct pci_dev *pdev)
{
	pci_set_drvdata(pdev, NULL);
}

static const struct pci_device_id cavium_rst_id_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_CAVIUM, PCI_DEVICE_ID_CAVIUM_RST) },
	{ 0, }
};

static struct pci_driver cavium_rst_driver = {
	.name = DRV_NAME,
	.id_table = cavium_rst_id_table,
	.probe = cavium_rst_probe,
	.remove = cavium_rst_remove,
};

static int __init cavium_rst_init_module(void)
{
	return pci_register_driver(&cavium_rst_driver);
}

static void __exit cavium_rst_cleanup_module(void)
{
	pci_unregister_driver(&cavium_rst_driver);
}

module_init(cavium_rst_init_module);
module_exit(cavium_rst_cleanup_module);

MODULE_DESCRIPTION(DRV_NAME);
MODULE_AUTHOR("Cavium Networks <support@cavium.com>");
MODULE_LICENSE("GPL v2");
MODULE_DEVICE_TABLE(pci, cavium_rst_id_table);
