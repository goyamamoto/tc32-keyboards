/*
 * The V65 V3's pads for the BLE and 2.4G deep sleep
 * (CONFIG_TLSR_BOARD_DEEP_SLEEP_PADS, called from ble_deep_sleep() with
 * interrupts locked): the V75 Pro's, without its mode switch. Whether a GPIO
 * output keeps its level through the deep sleep is not documented; the
 * analog pulls and the pad wake circuit do, and the pulls are chosen so that
 * each level the wake depends on is the same either way.
 *
 * - The 15 matrix columns: GPIO outputs at 0 with 100 kOhm pull-downs. The
 *   six rows PC0-PC5: inputs without pulls, each waking the chip on low, so a
 *   key, which joins its row to a column, wakes it.
 * - PD3, the RGB drivers' shutdown line (active low): at 0 with a 100 kOhm
 *   pull-down, so the drivers stay off.
 * - PA0 and PB6 (the V75 Pro's switch pins, not connected here): input and
 *   output off with 100 kOhm pull-downs. PA7 (SWS): input and output off
 *   with its 1 MOhm pull-up.
 * - The knob (PC6, PB7): inputs with 1 MOhm pull-ups, no wake.
 * - PD6 (charging) an input with a 10 kOhm pull-up, PD7 (power in) an input
 *   without pull, neither waking: the power switch moved to the cable cuts
 *   the battery off, which is a power-on; a key wakes the chip. PB3 (the
 *   battery ADC) and the USB pads PA5 and PA6: input and output off, no
 *   pull, and the USB D+ pull-up (analog 0x0b bit 7) off, as USB is not
 *   served in the sleep.
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

/* Per port A-D. */
static const uint8_t oen[4] = {0xe1, 0xc8, 0x7f, 0xc0}; /* outputs: PA1-4; PB0-2, PB4-5; PC7; PD0-5 */
static const uint8_t ie[4] = {0x00, 0x80, 0x7f, 0xc0};  /* inputs: PB7; PC0-6; PD6-7 */
static const uint8_t pulls[8] = {0xaa, 0x42, 0x2a, 0x6a, 0x00, 0x90, 0xaa, 0x3a};
/* The rows on low. */
static const uint8_t wake_pol[4] = {0x00, 0x00, 0x3f, 0x00};
static const uint8_t wake_en[4] = {0x00, 0x00, 0x3f, 0x00};

void board_deep_sleep_pads(void)
{
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
	for (uint8_t p = 0; p < 4U; p++) {
		tlsr_analog_write(ANA_WAKE_POL + p, wake_pol[p]);
		tlsr_analog_write(ANA_WAKE_EN + p, wake_en[p]);
	}
}
