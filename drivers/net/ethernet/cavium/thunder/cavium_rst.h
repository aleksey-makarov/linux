/*
 * cavium_rst.h - Rest and Fuses (RST) Cavium hardware
 *
 * Copyright (c) 2017 Cavium, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 */

#ifndef CAVIUM_RST_H
#define CAVIUM_RST_H

struct cavium_rst;

struct cavium_rst *cavium_rst_get(void);
void cavium_rst_put(struct cavium_rst *rst);

u64 cavium_rst_get_clock_rate(struct cavium_rst *rst);
extern const u64 cavium_rst_clock_rate_default;

#endif
