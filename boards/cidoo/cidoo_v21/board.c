/*
 * CIDOO V21: pads the keymap does not use.
 * - PA2-PA4 and PB0-PB2 carry the backlight PWM. Until the backlight driver
 *   starts (src/led_key_matrix.c, POST_KERNEL) they are GPIO outputs driven
 *   low (the backlight off), not floating.
 * - PB6 and PC4 read the mode switch (the BLE stack reads them at boot and
 *   then every 50 ms with power in, every second on the battery, with their
 *   10 kOhm pull-ups on only for each reading; src/ble/ble.c), and PD6 and
 *   PD7 are the charger's status and power in (the battery command, and in
 *   the BT and 2.4G positions the BLE stack's level and switch watch, make
 *   them inputs, PD6 with its 10 kOhm pull-up, at their first use). Until
 *   then: GPIO function with output and input disabled (DS-TLSR8278 7.1.1:
 *   unused pads need IE = 0). PC's input enable is an analog register
 *   (afe_0xc0).
 *
 * PA7, the SWS pin on the V75 Pro, is a matrix column here: the kscan driver
 * makes it a GPIO output, so there is no SWS on a V21 running this firmware.
 *
 * Copyright (c) 2026 Go Yamamoto
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <zephyr/init.h>
#include <zephyr/irq.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/sys/util.h>

#include "tlsr_slots.h"
#include "tlsr_spi_flash.h"

#define GPIO_PORT(port) (0x00800580U + 8U * (port))
#define GPIO_IE         1U /* PA, PB, PD, PE on the TLSR8278; PC's is analog */
#define GPIO_OEN        2U /* 1: output disabled */
#define GPIO_OUT        3U
#define GPIO_FUNC       6U /* 1: act as GPIO */

#define PORT_A 0U
#define PORT_B 1U
#define PORT_D 3U
#define PC_IE_UNUSED BIT(4) /* PC4 */

#define ANALOG_PORT_ADDR  0x008000b8U
#define ANALOG_PORT_DATA  0x008000b9U
#define ANALOG_PORT_CTRL  0x008000baU
#define ANALOG_PORT_BUSY  BIT(0)
#define ANALOG_PORT_WRITE BIT(5)
#define ANALOG_PORT_START BIT(6)
#define ANA_PC_IE     0xc0U /* PC's input enable (1: on), one bit per pin */

static const struct {
	uint8_t port;
	uint8_t pins;  /* GPIO, input and output off */
	uint8_t low;   /* GPIO, driven low, input off */
} unused[] = {
	{PORT_A, 0U, BIT(2) | BIT(3) | BIT(4)},
	{PORT_B, BIT(6), BIT(0) | BIT(1) | BIT(2)},
	{PORT_D, BIT(6) | BIT(7), 0U},
};

/*
 * The analog register port (0xb8-0xba) is not documented in DS-TLSR8278: the
 * code waits for bit 0 of 0xba to read 0, for at most 20000 polls (over a
 * millisecond). This runs at PRE_KERNEL_1, before the boot guard's watchdog,
 * so a port that stays busy reboots (0x6f bit 5, the whole-chip software
 * reset, as the SoC code does) instead of hanging the boot.
 */
#define ANALOG_PORT_TRIES 20000U
#define PWDNEN            0x0080006fU
#define PWDNEN_RESET_ALL  BIT(5)

static void analog_wait(void)
{
	for (uint32_t i = 0; i < ANALOG_PORT_TRIES; i++) {
		if ((sys_read8(ANALOG_PORT_CTRL) & ANALOG_PORT_BUSY) == 0U) {
			return;
		}
	}
	sys_write8(PWDNEN_RESET_ALL, PWDNEN);
	for (;;) {
	}
}

static uint8_t analog_read(uint8_t addr)
{
	sys_write8(addr, ANALOG_PORT_ADDR);
	sys_write8(ANALOG_PORT_START, ANALOG_PORT_CTRL);
	analog_wait();
	uint8_t v = sys_read8(ANALOG_PORT_DATA);

	sys_write8(0U, ANALOG_PORT_CTRL);
	return v;
}

static void analog_write(uint8_t addr, uint8_t value)
{
	sys_write8(addr, ANALOG_PORT_ADDR);
	sys_write8(value, ANALOG_PORT_DATA);
	sys_write8(ANALOG_PORT_START | ANALOG_PORT_WRITE, ANALOG_PORT_CTRL);
	analog_wait();
	sys_write8(0U, ANALOG_PORT_CTRL);
}

static int cidoo_v21_pads_init(void)
{
	unsigned int key = irq_lock();

	for (size_t i = 0; i < ARRAY_SIZE(unused); i++) {
		mm_reg_t base = GPIO_PORT(unused[i].port);
		uint8_t pins = unused[i].pins;
		uint8_t low = unused[i].low;

		/* The level first, then GPIO, then the output: no high glitch on the LEDs. */
		sys_write8(sys_read8(base + GPIO_OUT) & (uint8_t)~low, base + GPIO_OUT);
		sys_write8(sys_read8(base + GPIO_OEN) | pins, base + GPIO_OEN);
		sys_write8(sys_read8(base + GPIO_IE) & (uint8_t)~(pins | low), base + GPIO_IE);
		sys_write8(sys_read8(base + GPIO_FUNC) | pins | low, base + GPIO_FUNC);
		sys_write8(sys_read8(base + GPIO_OEN) & (uint8_t)~low, base + GPIO_OEN);
	}
	analog_write(ANA_PC_IE, analog_read(ANA_PC_IE) & (uint8_t)~PC_IE_UNUSED);
	irq_unlock(key);
	return 0;
}

SYS_INIT(cidoo_v21_pads_init, PRE_KERNEL_1, 0);

#ifdef CONFIG_TLSR_BOOT_GUARD_EARLY
/*
 * Right after the SoC setup (analog, crystal, 48 MHz, the system timer the
 * flash waits need) and before any device: the boot guard counts this
 * boot and goes back to the other image after too many, so that a hang
 * anywhere later, even before the kernel, is counted (tlsr_boot_guard.c).
 */
void board_early_init_hook(void)
{
	/* The flash supply trims first, before the boot guard writes the flash. */
	tlsr_spi_flash_trim_calib();
	tlsr_boot_guard_early();
}
#endif
