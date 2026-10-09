/*
 * The Cidoo V75 Pro's status display (CONFIG_STATUS_DISPLAY_CIDOO_V75PRO): what the
 * snapshot of status_display.c shows on which LEDs, through the backlight
 * driver's overlay (led_driver_spi.c), at a fixed brightness whatever the
 * backlight's setting, with the backlight's own frame under it (dark when
 * the backlight is off).
 *
 * - The three LEDs left of the knob: the battery, as a bar of one to three
 *   from the first; green from 50 %, orange from 20 %, red under it; blue
 *   while charging, and all three green once charging is complete with the
 *   cable in. Off when there is no measurement.
 * - Q, W, E: the Bluetooth profiles 1-3 (the standard assignment of the
 *   channel keys): the selected profile green when a central is connected
 *   with the report's notifications on, white while it has no bond and
 *   advertises for a pairing, orange while it looks for its bonded host;
 *   any other profile dim blue when it has a bond; off without one. Outside
 *   the BT position only the dim blue of the bonds.
 * - R: the 2.4G link: green when linked, white while pairing, orange while
 *   it looks for the dongle; off outside the 2.4G position.
 * - 1 to 0: the battery level, one key per ten percent started, in the
 *   bar's colour.
 * - F1 to F10: the CPU left over for the lowest-priority thread, one key
 *   per ten percent started, cyan; off until measured.
 * - A and S: the Windows or the Mac mode (Fn + A and Fn + S):
 *   the one in use white (the kept layer of layer_keep.c is the Mac one).
 *
 * Copyright (c) 2026 Go Yamamoto
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include "status_display.h"
#include "v75pro_status.h"

/* An LED: its driver chip (1 or 2) and the channels of its red, green and blue. */
struct led {
	uint8_t chip, r, g, b;
};

enum {
	LED_Q, LED_W, LED_E, LED_R,
	LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7, LED_8, LED_9, LED_0,
	LED_F1, LED_F2, LED_F3, LED_F4, LED_F5, LED_F6, LED_F7, LED_F8, LED_F9, LED_F10,
	LED_A, LED_S,
	LED_KNOB0, LED_KNOB1, LED_KNOB2,
	LEDS
};

static const struct led leds[LEDS] = {
	[LED_Q] = {1, 49, 81, 65}, [LED_W] = {1, 50, 82, 66}, [LED_E] = {1, 51, 83, 67},
	[LED_R] = {1, 52, 84, 68},
	[LED_1] = {1, 1, 33, 17},  [LED_2] = {1, 2, 34, 18},  [LED_3] = {1, 3, 35, 19},
	[LED_4] = {1, 4, 36, 20},  [LED_5] = {1, 5, 37, 21},  [LED_6] = {1, 6, 38, 22},
	[LED_7] = {1, 7, 39, 23},  [LED_8] = {1, 8, 40, 24},  [LED_9] = {1, 9, 41, 25},
	[LED_0] = {1, 10, 42, 26},
	[LED_F1] = {2, 2, 34, 18},  [LED_F2] = {2, 3, 35, 19},  [LED_F3] = {2, 4, 36, 20},
	[LED_F4] = {2, 5, 37, 21},  [LED_F5] = {2, 6, 38, 22},  [LED_F6] = {2, 7, 39, 23},
	[LED_F7] = {2, 8, 40, 24},  [LED_F8] = {2, 9, 41, 25},  [LED_F9] = {2, 10, 42, 26},
	[LED_F10] = {2, 11, 43, 27},
	[LED_A] = {1, 97, 129, 113}, [LED_S] = {1, 98, 130, 114},
	[LED_KNOB0] = {2, 96, 128, 112}, [LED_KNOB1] = {2, 98, 130, 114}, [LED_KNOB2] = {2, 99, 131, 115},
};

enum colour { OFF, GREEN, ORANGE, RED, BLUE, DIM_BLUE, WHITE, CYAN };

static const uint8_t palette[][3] = {
	[OFF] = {0, 0, 0},       [GREEN] = {0, 160, 0},    [ORANGE] = {200, 70, 0},
	[RED] = {200, 0, 0},     [BLUE] = {0, 0, 200},     [DIM_BLUE] = {0, 0, 40},
	[WHITE] = {120, 120, 120}, [CYAN] = {0, 140, 160},
};

static uint8_t shown[LEDS]; /* a colour per LED */
static bool active;

static void battery(const struct status_snapshot *s)
{
	uint8_t bar = 0, keys = 0;
	enum colour c = OFF;

	if (s->battery_known) {
		uint8_t pct = s->battery_percent;

		bar = pct >= 67U ? 3U : (pct >= 34U ? 2U : 1U);
		keys = (uint8_t)MIN((pct + 9U) / 10U, 10U);
		c = pct >= 50U ? GREEN : (pct >= 20U ? ORANGE : RED);
		if (s->charging) {
			c = BLUE;
		} else if (s->power_in) {
			bar = 3U;
			keys = 10U;
			c = GREEN;
		}
	}
	for (uint8_t i = 0; i < 3U; i++) {
		shown[LED_KNOB0 + i] = i < bar ? c : OFF;
	}
	for (uint8_t i = 0; i < 10U; i++) {
		shown[LED_1 + i] = i < keys ? c : OFF;
	}
}

static void profiles(const struct status_snapshot *s)
{
	for (uint8_t p = 0; p < 3U; p++) {
		enum colour c = (s->ble_bonded & BIT(p)) ? DIM_BLUE : OFF;

		if (s->link == STATUS_LINK_BLE && p == s->ble_active) {
			if (s->ble_connected && s->ble_ready) {
				c = GREEN;
			} else if (s->ble_pairing) {
				c = WHITE;
			} else {
				c = ORANGE;
			}
		}
		shown[LED_Q + p] = c;
	}
	shown[LED_R] = s->link != STATUS_LINK_P24 ? OFF
		       : (s->p24_linked ? GREEN : (s->p24_pairing ? WHITE : ORANGE));
}

static void cpu(const struct status_snapshot *s)
{
	uint8_t n = s->cpu_left == 0xffU ? 0U : (uint8_t)MIN((s->cpu_left + 9U) / 10U, 10U);

	for (uint8_t i = 0; i < 10U; i++) {
		shown[LED_F1 + i] = i < n ? CYAN : OFF;
	}
}

static void mode(const struct status_snapshot *s)
{
	shown[LED_A] = s->mode_known && !s->mac_mode ? WHITE : OFF;
	shown[LED_S] = s->mode_known && s->mac_mode ? WHITE : OFF;
}

void status_board_show(const struct status_snapshot *s)
{
	battery(s);
	profiles(s);
	cpu(s);
	mode(s);
	active = true;
	v75pro_backlight_refresh();
}

void status_board_hide(void)
{
	active = false;
	v75pro_backlight_refresh();
}

bool v75pro_status_active(void)
{
	return active;
}

int v75pro_status_channel(uint8_t chip, uint8_t channel)
{
	if (!active) {
		return -1;
	}
	for (uint8_t i = 0; i < LEDS; i++) {
		const struct led *l = &leds[i];

		if (l->chip != chip) {
			continue;
		}
		if (channel == l->r) {
			return palette[shown[i]][0];
		}
		if (channel == l->g) {
			return palette[shown[i]][1];
		}
		if (channel == l->b) {
			return palette[shown[i]][2];
		}
	}
	return -1;
}
