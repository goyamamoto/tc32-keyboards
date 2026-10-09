/*
 * The V75 Pro's pads for the BLE and 2.4G deep sleep
 * (CONFIG_TLSR_BOARD_DEEP_SLEEP_PADS, called from ble_deep_sleep() with
 * interrupts locked). Whether a GPIO output keeps its level through the deep
 * sleep is not documented; the analog pulls and the pad wake circuit do. The
 * pulls are chosen so that each level the wake depends on is the same either
 * way, but for PA7: the switch's common pin is driven 0 against a 1 MOhm
 * pull-up, and if that drive did not hold, the switch pin joined to it would
 * read high in the sleep (in the BT position PA0 would then wake the chip at
 * once).
 *
 * - The 15 matrix columns: GPIO outputs at 0 with 100 kOhm pull-downs. The
 *   six rows PC0-PC5: inputs without pulls, each waking the chip on low, so a
 *   key, which joins its row to a column, wakes it.
 * - PD3, the RGB drivers' shutdown line (active low): at 0 with a 100 kOhm
 *   pull-down, so the drivers stay off.
 * - The mode switch: PA7, its common pin, a GPIO output at 0 with a 1 MOhm
 *   pull-up; PA0 (BT) and PB6 (2.4G) inputs with 10 kOhm pull-ups. The
 *   switch joins one of them to PA7. Each wakes the chip on the level
 *   opposite to the one it reads before the sleep, so moving the switch
 *   wakes it in either position. With PA7's drive holding, about 0.33 mA
 *   flows for the whole sleep through the joined pin's 10 kOhm pull-up into
 *   PA7; a weaker pull-up would cut that, but its wake on a switch move has
 *   not been checked.
 * - The knob (PC6, PB7): inputs with 1 MOhm pull-ups, no wake.
 * - PD6 (charging) an input with a 10 kOhm pull-up, PD7 (power in) an input
 *   without pull, neither waking: plugging USB in does not wake the chip, a
 *   key or the switch does. PB3 (the battery ADC) and the USB pads PA5 and
 *   PA6: input and output off, no pull, and the USB D+ pull-up (analog 0x0b
 *   bit 7) off, as USB is not served in the sleep.
 *
 * The registers are written whole per port: the GPIO function, output
 * enable, output and input enable, the pulls (analog 0x0e-0x15), and the
 * wake polarity (analog 0x21-0x24, a bit set wakes on low) and enable
 * (analog 0x27-0x2a).
 *
 * Copyright (c) 2026 Go Yamamoto
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/sys/util.h>

#include "ble/tlsr_analog.h"

#define GPIO_PORT0         0x00800580U
#define GPIO_IN            0U
#define GPIO_IE            1U /* PA, PB, PD; port C's is analog 0xc0 */
#define GPIO_OEN           2U /* 1: output disabled */
#define GPIO_OUT           3U
#define GPIO_FUNC          6U /* 1: GPIO */
#define ANA_PC_IE          0xc0U
#define ANA_PULL0          0x0eU
#define ANA_WAKE_POL       0x21U
#define ANA_WAKE_EN        0x27U
#define ANA_USB_DP         0x0bU
#define ANA_USB_DP_PULLUP  BIT(7) /* the 1.5 kOhm pull-up on D+ (PA6) */
#define PORT_A             0U
#define PORT_B             1U
#define SWITCH_BT          BIT(0) /* PA0 */
#define SWITCH_24G         BIT(6) /* PB6 */

/* Per port A-D. */
static const uint8_t oen[4] = {0x61, 0xc8, 0x7f, 0xc0}; /* outputs: PA1-4, PA7; PB0-2, PB4-5; PC7; PD0-5 */
static const uint8_t ie[4] = {0x01, 0xc0, 0x7f, 0xc0};  /* inputs: PA0; PB6-7; PC0-6; PD6-7 */
static const uint8_t pulls[8] = {0xab, 0x42, 0x2a, 0x7a, 0x00, 0x90, 0xaa, 0x3a};
/* The rows on low; the switch pins' polarity is set from their levels (below). */
static const uint8_t wake_pol[4] = {0x00, 0x00, 0x3f, 0x00};
static const uint8_t wake_en[4] = {SWITCH_BT, SWITCH_24G, 0x3f, 0x00};

static bool reads_high(uint32_t port, uint8_t bit)
{
	return (sys_read8(GPIO_PORT0 + 8U * port + GPIO_IN) & bit) != 0U;
}

void board_deep_sleep_pads(void)
{
	uint8_t pol[4];

	for (uint32_t p = 0; p < 4U; p++) {
		uint32_t b = GPIO_PORT0 + 8U * p;

		/* The outputs' level first, then the GPIO function, then the output enables. */
		sys_write8(0x00, b + GPIO_OUT);
		sys_write8(0xff, b + GPIO_FUNC);
		sys_write8(oen[p], b + GPIO_OEN);
		if (p != 2U) {
			sys_write8(ie[p], b + GPIO_IE);
		}
	}
	tlsr_analog_write(ANA_PC_IE, ie[2]);
	tlsr_analog_write(ANA_USB_DP, tlsr_analog_read(ANA_USB_DP) & (uint8_t)~ANA_USB_DP_PULLUP);
	for (uint8_t r = 0; r < ARRAY_SIZE(pulls); r++) {
		tlsr_analog_write(ANA_PULL0 + r, pulls[r]);
	}
	/* 10 ms for the levels to settle before the wake pads are armed. */
	k_busy_wait(10000);
	for (uint32_t p = 0; p < 4U; p++) {
		pol[p] = wake_pol[p];
	}
	/* Each switch pin wakes on the level opposite to the one it reads now. */
	if (reads_high(PORT_A, SWITCH_BT)) {
		pol[PORT_A] |= SWITCH_BT;
	}
	if (reads_high(PORT_B, SWITCH_24G)) {
		pol[PORT_B] |= SWITCH_24G;
	}
	for (uint8_t p = 0; p < 4U; p++) {
		tlsr_analog_write(ANA_WAKE_POL + p, pol[p]);
		tlsr_analog_write(ANA_WAKE_EN + p, wake_en[p]);
	}
}
