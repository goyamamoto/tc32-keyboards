/*
 * CIDOO V75 Pro: pads the keymap does not use are GPIO function with output
 * and input disabled (DS-TLSR8278 7.1.1: unused pads need IE = 0). PA0 and
 * PB6 read the mode switch, PB3 the battery voltage, PD6 and PD7 the
 * charging and power-in lines; the BLE build's code sets each while it reads
 * it. PD7's reset function is SPI_CK, which would drive the pin.
 *
 * PA7 is the SWS pin, kept for recovery: its function is left alone, and it
 * gets the 1 MOhm pull-up, since a pad with its input enabled must not float
 * (DS-TLSR8278 7.1.1).
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
#define GPIO_FUNC       6U /* 1: act as GPIO */

#define PORT_A 0U
#define PORT_B 1U
#define PORT_D 3U

#define ANALOG_PORT_ADDR  0x008000b8U
#define ANALOG_PORT_DATA  0x008000b9U
#define ANALOG_PORT_CTRL  0x008000baU
#define ANALOG_PORT_BUSY  BIT(0)
#define ANALOG_PORT_WRITE BIT(5)
#define ANALOG_PORT_START BIT(6)
#define ANA_PULL_PA47 0x0fU /* PA4..PA7 pull-ups, 2 bits each (DS-TLSR8278 Table 7-5) */
#define PA7_PULL_MASK (BIT(7) | BIT(6))
#define PA7_PULL_1M   BIT(6)

static const struct {
	uint8_t port;
	uint8_t pins;
} unused[] = {
	{PORT_A, BIT(0)},
	{PORT_B, BIT(3) | BIT(6)},
	{PORT_D, BIT(6) | BIT(7)},
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

static int cidoo_v75pro_pads_init(void)
{
	unsigned int key = irq_lock();

	for (size_t i = 0; i < ARRAY_SIZE(unused); i++) {
		mm_reg_t base = GPIO_PORT(unused[i].port);
		uint8_t pins = unused[i].pins;

		sys_write8(sys_read8(base + GPIO_OEN) | pins, base + GPIO_OEN);
		sys_write8(sys_read8(base + GPIO_IE) & (uint8_t)~pins, base + GPIO_IE);
		sys_write8(sys_read8(base + GPIO_FUNC) | pins, base + GPIO_FUNC);
	}
	analog_write(ANA_PULL_PA47,
		     (analog_read(ANA_PULL_PA47) & (uint8_t)~PA7_PULL_MASK) | PA7_PULL_1M);
	irq_unlock(key);
	return 0;
}

SYS_INIT(cidoo_v75pro_pads_init, PRE_KERNEL_1, 0);

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
