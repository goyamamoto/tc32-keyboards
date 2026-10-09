/*
 * The V75 Pro's status display and the backlight driver: the display's values
 * for the frame (v75pro_status.c), the driver's refresh (led_driver_spi.c).
 *
 * Copyright (c) 2026 Go Yamamoto
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef TC32_V75PRO_STATUS_H_
#define TC32_V75PRO_STATUS_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef CONFIG_STATUS_DISPLAY_CIDOO_V75PRO
/* The display is on: the chips show a frame whatever the backlight's setting. */
bool v75pro_status_active(void);
/* The display's value of a chip's channel (chip 1 or 2), or -1 for the backlight's own. */
int v75pro_status_channel(uint8_t chip, uint8_t channel);
#else
static inline bool v75pro_status_active(void)
{
	return false;
}
static inline int v75pro_status_channel(uint8_t chip, uint8_t channel)
{
	(void)chip;
	(void)channel;
	return -1;
}
#endif

/* The driver: the next frame now, and the chips started or stopped as the display and the setting want. */
void v75pro_backlight_refresh(void);

#endif /* TC32_V75PRO_STATUS_H_ */
