/*
 * This board's part of the status display, for the LED driver (src/led_driver_spi.c):
 * whether the display is on, and the colour byte it shows on a PWM channel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TC32_LED_DRIVER_STATUS_H_
#define TC32_LED_DRIVER_STATUS_H_

#include "v65v3_status.h"

#define board_status_active  v65v3_status_active
#define board_status_channel v65v3_status_channel

#endif
